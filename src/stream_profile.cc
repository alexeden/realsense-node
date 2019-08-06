#ifndef STREAM_PROFILE_H
#define STREAM_PROFILE_H

#include "stream_profile.cc"
#include "utils.cc"
#include <iostream>
#include <librealsense2/h/rs_internal.h>
#include <librealsense2/hpp/rs_types.hpp>
#include <librealsense2/rs.h>
#include <napi.h>

using namespace Napi;

class RSStreamProfile : public ObjectWrap<RSStreamProfile> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSSyncer",
		  {
			InstanceMethod("destroy", &RSStreamProfile::Destroy),
			InstanceMethod("stream", &RSStreamProfile::Stream),
			InstanceMethod("format", &RSStreamProfile::Format),
			InstanceMethod("fps", &RSStreamProfile::Fps),
			InstanceMethod("index", &RSStreamProfile::Index),
			InstanceMethod("uniqueID", &RSStreamProfile::UniqueID),
			InstanceMethod("isDefault", &RSStreamProfile::IsDefault),
			InstanceMethod("isVideoProfile", &RSStreamProfile::IsVideoProfile),
			InstanceMethod("isMotionProfile", &RSStreamProfile::IsMotionProfile),
			InstanceMethod("width", &RSStreamProfile::Width),
			InstanceMethod("height", &RSStreamProfile::Height),
			InstanceMethod("getExtrinsicsTo", &RSStreamProfile::GetExtrinsicsTo),
			InstanceMethod("getVideoStreamIntrinsics", &RSStreamProfile::GetVideoStreamIntrinsics),
			InstanceMethod("getMotionIntrinsics", &RSStreamProfile::GetMotionIntrinsics),
		  });
		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSStreamProfile", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_stream_profile* p, bool own = false) {
		Nan::EscapableHandleScope scope;

		static FunctionReference constructor;
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
		this->profile_				   = p;
		this->own_profile_			   = own;
		auto p_args = { &this->stream_, &this->format_, &this->index_, &this->unique_id_, &this->fps_, &this->error_ };
		CallNativeFunc(rs2_get_stream_profile_data, &this->error_, p, p_args...);
		this->is_default_ = rs2_is_stream_profile_default(p, &this->error_);
		if (GetNativeResult<
			  bool>(rs2_stream_profile_is, &this->error_, p, RS2_EXTENSION_VIDEO_PROFILE, &this->error_)) {
			this->is_video_ = true;
			CallNativeFunc(rs2_get_video_stream_resolution, &this->error_, p, &this->width_, &this->height_, &this->error_);
		}
		else if (GetNativeResult<
				   bool>(rs2_stream_profile_is, &this->error_, p, RS2_EXTENSION_MOTION_PROFILE, &this->error_)) {
			this->is_motion_ = true;
		}

		return scope.Escape(instance);
	}

  private:
	RSStreamProfile(const CallbackInfo& info)
	  : ObjectWrap<RSStreamProfile>(info)
	  , error_(nullptr)
	  , profile_(nullptr)
	  , index_(0)
	  , unique_id_(0)
	  , fps_(0)
	  , format_(static_cast<rs2_format>(0))
	  , stream_(static_cast<rs2_stream>(0))
	  , is_video_(false)
	  , width_(0)
	  , height_(0)
	  , is_default_(false)
	  , own_profile_(false)
	  , is_motion_(false) {
	}

	~RSStreamProfile() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (profile_ && own_profile_) rs2_delete_stream_profile(profile_);
		profile_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	Napi::Value Stream(const CallbackInfo& info) {
		return Number::New(info.Env(), this->stream_);
	}
	Napi::Value Format(const CallbackInfo& info) {
		return Number::New(info.Env(), this->format_);
	}
	Napi::Value Fps(const CallbackInfo& info) {
		return Number::New(info.Env(), this->fps_);
	}
	Napi::Value Index(const CallbackInfo& info) {
		return Number::New(info.Env(), this->index_);
	}
	Napi::Value UniqueID(const CallbackInfo& info) {
		return Number::New(info.Env(), this->unique_id_);
	}
	Napi::Value IsDefault(const CallbackInfo& info) {
		return Boolean::New(info.Env(), this->is_default_);
	}
	Napi::Value IsVideoProfile(const CallbackInfo& info) {
		return Boolean::New(info.Env(), this->is_video_);
	}
	Napi::Value IsMotionProfile(const CallbackInfo& info) {
		return Boolean::New(info.Env(), this->is_motion_);
	}
	Napi::Value Width(const CallbackInfo& info) {
		return Number::New(info.Env(), this->width_);
	}
	Napi::Value Height(const CallbackInfo& info) {
		return Number::New(info.Env(), this->height_);
	}

	Napi::Value GetExtrinsicsTo(const CallbackInfo& info) {
		auto to = ObjectWrap<RSStreamProfile>::Unwrap(info[0].ToObject());
		if (!to) return info.Env().Undefined();

		rs2_extrinsics res;
		CallNativeFunc(rs2_get_extrinsics, &this->error_, this->profile_, to->profile_, &res, &this->error_);
		if (this->error_) return info.Env().Undefined();

		RSExtrinsics rsres(res);
		info.GetReturnValue().Set(rsres.GetObject());
	}

	Napi::Value GetVideoStreamIntrinsics(const CallbackInfo& info) {
		if (!this) return info.Env().Undefined();

		rs2_intrinsics intr;
		CallNativeFunc(rs2_get_video_stream_intrinsics, &this->error_, this->profile_, &intr, &this->error_);
		if (this->error_) return info.Env().Undefined();

		RSIntrinsics res(intr);
		info.GetReturnValue().Set(res.GetObject());
	}

	Napi::Value GetMotionIntrinsics(const CallbackInfo& info) {
		if (!this) return info.Env().Undefined();

		rs2_motion_device_intrinsic output;
		CallNativeFunc(rs2_get_motion_intrinsics, &this->error_, this->profile_, &output, &this->error_);
		if (this->error_) return info.Env().Undefined();

		RSMotionIntrinsics intrinsics(&output);
		info.GetReturnValue().Set(intrinsics.GetObject());
	}

  private:
	static FunctionReference constructor;

	rs2_error* error_;
	rs2_stream_profile* profile_;
	int32_t index_;
	int32_t unique_id_;
	int32_t fps_;
	rs2_format format_;
	rs2_stream stream_;
	bool is_video_;
	int32_t width_;
	int32_t height_;
	bool is_default_;
	bool own_profile_;
	bool is_motion_;

	friend class RSSensor;
};

Napi::FunctionReference RSStreamProfile::constructor;

#endif
