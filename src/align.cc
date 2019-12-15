#ifndef ALIGN_H
#define ALIGN_H

#include "frame_callbacks.cc"
#include "frameset.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSAlign : public ObjectWrap<RSAlign> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSAlign",
		  {
			InstanceMethod("destroy", &RSAlign::Destroy),
			InstanceMethod("process", &RSAlign::Process),
			InstanceMethod("waitForFrames", &RSAlign::WaitForFrames),
		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSAlign", func);

		return exports;
	}

	RSAlign(const CallbackInfo& info)
	  : ObjectWrap<RSAlign>(info)
	  , align_(nullptr)
	  , frame_queue_(nullptr)
	  , error_(nullptr) {
		auto stream	 = static_cast<rs2_stream>(info[0].ToNumber().Int32Value());
		this->align_ = GetNativeResult<rs2_processing_block*>(rs2_create_align, &this->error_, stream, &this->error_);

		this->frame_queue_ = GetNativeResult<rs2_frame_queue*>(rs2_create_frame_queue, &this->error_, 1, &this->error_);
		if (!this->frame_queue_) return;

		auto callback = new FrameCallbackForFrameQueue(this->frame_queue_);
		CallNativeFunc(rs2_start_processing, &this->error_, this->align_, callback, &this->error_);
	}

	~RSAlign() {
		DestroyMe();
	}

  private:
	friend class RSPipeline;

	static FunctionReference constructor;

	rs2_processing_block* align_;
	rs2_frame_queue* frame_queue_;
	rs2_error* error_;

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (align_) rs2_delete_processing_block(align_);
		align_ = nullptr;
		if (frame_queue_) rs2_delete_frame_queue(frame_queue_);
		frame_queue_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.This();
	}

	Napi::Value Process(const CallbackInfo& info) {
		auto frameset  = ObjectWrap<RSFrameSet>::Unwrap(info[0].ToObject());
		auto target_fs = ObjectWrap<RSFrameSet>::Unwrap(info[1].ToObject());
		if (!frameset || !target_fs) return Boolean::New(info.Env(), false);

		// rs2_process_frame will release the input frame, so we need to addref
		CallNativeFunc(rs2_frame_add_ref, &this->error_, frameset->GetFrames(), &this->error_);
		if (this->error_) return Boolean::New(info.Env(), false);

		CallNativeFunc(rs2_process_frame, &this->error_, this->align_, frameset->GetFrames(), &this->error_);
		if (this->error_) return Boolean::New(info.Env(), false);

		rs2_frame* frame = nullptr;
		auto ret_code
		  = GetNativeResult<int>(rs2_poll_for_frame, &this->error_, this->frame_queue_, &frame, &this->error_);
		if (!ret_code) return Boolean::New(info.Env(), false);

		target_fs->Replace(frame);
		return Boolean::New(info.Env(), true);
	}

	Napi::Value WaitForFrames(const CallbackInfo& info) {
		rs2_frame* result
		  = GetNativeResult<rs2_frame*>(rs2_wait_for_frame, &this->error_, this->frame_queue_, 5000, &this->error_);
		if (!result) return info.Env().Undefined();

		return RSFrameSet::NewInstance(info.Env(), result);
	}
};

Napi::FunctionReference RSAlign::constructor;

#endif
