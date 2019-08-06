#ifndef FRAMESET_H
#define FRAMESET_H

#include "frame.cc"
#include "stream_profile_extractor.cc"
#include "utils.cc"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>
using namespace Napi;

class RSFrameSet : public ObjectWrap<RSFrameSet> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSFrameSet",
		  {
			InstanceMethod("destroy", &RSFrameSet::Destroy),
			InstanceMethod("getSize", &RSFrameSet::GetSize),
			InstanceMethod("getFrame", &RSFrameSet::GetFrame),
			InstanceMethod("replaceFrame", &RSFrameSet::ReplaceFrame),
			InstanceMethod("indexToStream", &RSFrameSet::IndexToStream),
			InstanceMethod("indexToStreamIndex", &RSFrameSet::IndexToStreamIndex),
		  });
		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSFrameSet", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_frame* frame) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});
		auto unwrapped  = ObjectWrap<RSFrameSet>::Unwrap(instance);
		unwrapped->SetFrame(frame);

		return scope.Escape(napi_value(instance)).ToObject();
	}

	rs2_frame* GetFrames() {
		return frames_;
	}

	void Replace(rs2_frame* frame) {
		DestroyMe();
		SetFrame(frame);
	}

  private:
	RSFrameSet(const CallbackInfo& info)
	  : ObjectWrap<RSFrameSet>(info) {
		error_  = nullptr;
		frames_ = nullptr;
	}

	~RSFrameSet() {
		DestroyMe();
	}

	void SetFrame(rs2_frame* frame) {
		if (
		  !frame
		  || (!GetNativeResult<
			  int>(rs2_is_frame_extendable_to, &error_, frame, RS2_EXTENSION_COMPOSITE_FRAME, &error_)))
			return;

		frames_		 = frame;
		frame_count_ = GetNativeResult<int>(rs2_embedded_frames_count, &error_, frame, &error_);
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (frames_) rs2_release_frame(frames_);
		frames_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		auto unwrapped = ObjectWrap<RSFrameSet>::Unwrap(info[0].As<Object>());
		if (unwrapped) { unwrapped->DestroyMe(); }
		return info.Env().Undefined();
	}

	Napi::Value GetSize(const CallbackInfo& info) {
		if (this->frames_) { return Number::New(info.Env(), this->frame_count_); }
		return Number::New(info.Env(), 0);
	}

	Napi::Value GetFrame(const CallbackInfo& info) {
		if (!this->frames_) return info.Env().Undefined();

		rs2_stream stream = static_cast<rs2_stream>(info[0].ToNumber().Int32Value());
		auto stream_index = info[1].ToNumber().Int32Value();
		// if RS2_STREAM_ANY is used, we return the first frame.
		if (stream == RS2_STREAM_ANY && this->frame_count_) {
			rs2_frame* frame
			  = GetNativeResult<rs2_frame*>(rs2_extract_frame, &this->error_, this->frames_, 0, &this->error_);
			if (!frame) return info.Env().Undefined();

			return RSFrame::NewInstance(info.Env(), frame);
		}

		for (uint32_t i = 0; i < this->frame_count_; i++) {
			rs2_frame* frame
			  = GetNativeResult<rs2_frame*>(rs2_extract_frame, &this->error_, this->frames_, i, &this->error_);
			if (!frame) continue;

			const rs2_stream_profile* profile = GetNativeResult<
			  const rs2_stream_profile*>(rs2_get_frame_stream_profile, &this->error_, frame, &this->error_);
			if (profile) {
				StreamProfileExtractor extrator(profile);
				if (
				  extrator.stream_ == stream && (!stream_index || (stream_index && stream_index == extrator.index_))) {
					return RSFrame::NewInstance(info.Env(), frame);
				}
			}
			rs2_release_frame(frame);
		}
	}

	Napi::Value ReplaceFrame(const CallbackInfo& info) {
		rs2_stream stream = static_cast<rs2_stream>(info[0].ToNumber().Int32Value());
		auto stream_index = info[1].ToNumber().Int32Value();
		auto target_frame = ObjectWrap<RSFrame>::Unwrap(info[2].ToObject());

		if (!this->frames_) return Boolean::New(info.Env(), false);

		for (uint32_t i = 0; i < this->frame_count_; i++) {
			rs2_frame* frame
			  = GetNativeResult<rs2_frame*>(rs2_extract_frame, &this->error_, this->frames_, i, &this->error_);
			if (!frame) continue;

			const rs2_stream_profile* profile = GetNativeResult<
			  const rs2_stream_profile*>(rs2_get_frame_stream_profile, &this->error_, frame, &this->error_);
			if (profile) {
				StreamProfileExtractor extrator(profile);
				if (
				  extrator.stream_ == stream && (!stream_index || (stream_index && stream_index == extrator.index_))) {
					target_frame->Replace(frame);
					return Boolean::New(info.Env(), true);
				}
			}
			rs2_release_frame(frame);
		}

		return Boolean::New(info.Env(), false);
	}

	Napi::Value IndexToStream(const CallbackInfo& info) {
		if (!this->frames_) return info.Env().Undefined();

		int32_t index = info[0].ToNumber().Int32Value();
		rs2_frame* frame
		  = GetNativeResult<rs2_frame*>(rs2_extract_frame, &this->error_, this->frames_, index, &this->error_);
		if (!frame) return info.Env().Undefined();

		const rs2_stream_profile* profile = GetNativeResult<
		  const rs2_stream_profile*>(rs2_get_frame_stream_profile, &this->error_, frame, &this->error_);
		if (!profile) {
			rs2_release_frame(frame);
			return info.Env().Undefined();
		}

		rs2_stream stream = RS2_STREAM_ANY;
		rs2_format format = RS2_FORMAT_ANY;
		int32_t fps		  = 0;
		int32_t idx		  = 0;
		int32_t unique_id = 0;
		CallNativeFunc(
		  rs2_get_stream_profile_data, &this->error_, profile, &stream, &format, &idx, &unique_id, &fps, &this->error_);
		rs2_release_frame(frame);
		if (this->error_) return info.Env().Undefined();

		return Number::New(info.Env(), stream);
	}

	Napi::Value IndexToStreamIndex(const CallbackInfo& info) {
		if (!this->frames_) return info.Env().Undefined();

		int32_t index = info[0].ToNumber().Int32Value();
		rs2_frame* frame
		  = GetNativeResult<rs2_frame*>(rs2_extract_frame, &this->error_, this->frames_, index, &this->error_);
		if (!frame) return info.Env().Undefined();

		const rs2_stream_profile* profile = GetNativeResult<
		  const rs2_stream_profile*>(rs2_get_frame_stream_profile, &this->error_, frame, &this->error_);
		if (!profile) {
			rs2_release_frame(frame);
			return info.Env().Undefined();
		}
		rs2_stream stream = RS2_STREAM_ANY;
		rs2_format format = RS2_FORMAT_ANY;
		int32_t fps		  = 0;
		int32_t idx		  = 0;
		int32_t unique_id = 0;
		CallNativeFunc(
		  rs2_get_stream_profile_data, &this->error_, profile, &stream, &format, &idx, &unique_id, &fps, &this->error_);
		rs2_release_frame(frame);
		if (this->error_) return info.Env().Undefined();

		return Number::New(info.Env(), idx);
	}

  private:
	static FunctionReference constructor;
	rs2_frame* frames_;
	uint32_t frame_count_;
	rs2_error* error_;
};

Napi::FunctionReference RSFrameSet::constructor;

#endif
