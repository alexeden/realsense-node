#ifndef SYNCER_H
#define SYNCER_H
#include "frame_callbacks.cc"
#include "frameset.cc"
#include "utils.cc"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSSyncer : public ObjectWrap<RSSyncer> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSSyncer",
		  {
			InstanceMethod("destroy", &RSSyncer::Destroy),
			InstanceMethod("waitForFrames", &RSSyncer::WaitForFrames),
			InstanceMethod("pollForFrames", &RSSyncer::PollForFrames),

		  });
		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSSyncer", func);

		return exports;
	}
	static Object NewInstance(Napi::Env env) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		return scope.Escape(napi_value(instance)).ToObject();
	}

  private:
	RSSyncer(const CallbackInfo& info)
	  : ObjectWrap<RSSyncer>(info)
	  , syncer_(nullptr)
	  , frame_queue_(nullptr)
	  , error_(nullptr) {
		this->syncer_
		  = GetNativeResult<rs2_processing_block*>(rs2_create_sync_processing_block, &this->error_, &this->error_);
		this->frame_queue_ = GetNativeResult<rs2_frame_queue*>(rs2_create_frame_queue, &this->error_, 1, &this->error_);
		auto callback	  = new FrameCallbackForFrameQueue(this->frame_queue_);
		CallNativeFunc(rs2_start_processing, &this->error_, this->syncer_, callback, &this->error_);
	}

	~RSSyncer() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (syncer_) rs2_delete_processing_block(syncer_);
		syncer_ = nullptr;
		if (frame_queue_) rs2_delete_frame_queue(frame_queue_);
		frame_queue_ = nullptr;
	}

	Napi::Value WaitForFrames(const CallbackInfo& info) {
		auto frameset = ObjectWrap<RSFrameSet>::Unwrap(info[0].ToObject());
		auto timeout  = info[1].ToNumber().Uint32Value();
		if (!frameset) return Boolean::New(info.Env(), false);

		rs2_frame* frames
		  = GetNativeResult<rs2_frame*>(rs2_wait_for_frame, &this->error_, this->frame_queue_, timeout, &this->error_);
		if (!frames) return Boolean::New(info.Env(), false);

		frameset->Replace(frames);
		return Boolean::New(info.Env(), true);
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();

		return info.Env().Undefined();
	}

	Napi::Value PollForFrames(const CallbackInfo& info) {
		auto frameset = ObjectWrap<RSFrameSet>::Unwrap(info[0].As<Object>());
		if (!frameset) return Boolean::New(info.Env(), false);

		rs2_frame* frame_ref = nullptr;
		auto res
		  = GetNativeResult<int>(rs2_poll_for_frame, &this->error_, this->frame_queue_, &frame_ref, &this->error_);
		if (!res) return Boolean::New(info.Env(), false);

		frameset->Replace(frame_ref);
		return Boolean::New(info.Env(), true);
	}

  private:
	static FunctionReference constructor;
	rs2_processing_block* syncer_;
	rs2_frame_queue* frame_queue_;
	rs2_error* error_;
	friend class RSSensor;
};

Napi::FunctionReference RSSyncer::constructor;

#endif
