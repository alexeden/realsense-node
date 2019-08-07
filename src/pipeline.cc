#ifndef PIPELINE_H
#define PIPELINE_H

#include "context.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSPipeline : public ObjectWrap<RSPipeline> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSPipeline",
		  {
			InstanceMethod("start", &RSPipeline::Start),
			InstanceMethod("startWithConfig", &RSPipeline::StartWithConfig),
			InstanceMethod("stop", &RSPipeline::Stop),
			InstanceMethod("waitForFrames", &RSPipeline::WaitForFrames),
			InstanceMethod("pollForFrames", &RSPipeline::PollForFrames),
			InstanceMethod("getActiveProfile", &RSPipeline::GetActiveProfile),
			InstanceMethod("create", &RSPipeline::Create),
			InstanceMethod("destroy", &RSPipeline::Destroy),
		  }
		);

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSPipeline", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		return scope.Escape(napi_value(instance)).ToObject();
	}

	friend class RSConfig;

	RSPipeline(const CallbackInfo& info)
	  : ObjectWrap<RSPipeline>(info)
	  , pipeline_(nullptr)
	  , error_(nullptr) {
	}

	~RSPipeline() {
		DestroyMe();
	}

  private:
	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (pipeline_) rs2_delete_pipeline(pipeline_);
		pipeline_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
	}

	Napi::Value Create(const CallbackInfo& info) {
		auto rsctx = ObjectWrap<RSContext>::Unwrap(info[0].ToObject());
		if (!rsctx) return info.Env().Undefined();

		this->pipeline_
		  = GetNativeResult<rs2_pipeline*>(rs2_create_pipeline, &this->error_, rsctx->ctx_, &this->error_);
	}

	Napi::Value StartWithConfig(const CallbackInfo& info) {
		if (!this->pipeline_) return info.Env().Undefined();

		RSConfig* config		   = ObjectWrap<RSConfig>::Unwrap(info[0].ToObject());
		rs2_pipeline_profile* prof = GetNativeResult<
		  rs2_pipeline_profile*>(rs2_pipeline_start_with_config, &this->error_, this->pipeline_, config->config_, &this->error_);
		if (!prof) return info.Env().Undefined();

		return RSPipelineProfile::NewInstance(info.Env(), prof);
	}

	Napi::Value Start(const CallbackInfo& info) {
		if (!this->pipeline_) return info.Env().Undefined();

		rs2_pipeline_profile* prof
		  = GetNativeResult<rs2_pipeline_profile*>(rs2_pipeline_start, &this->error_, this->pipeline_, &this->error_);
		if (!prof) return info.Env().Undefined();

		return RSPipelineProfile::NewInstance(info.Env(), prof);
	}

	Napi::Value Stop(const CallbackInfo& info) {
		if (!this->pipeline_) return info.Env().Undefined();

		CallNativeFunc(rs2_pipeline_stop, &this->error_, this->pipeline_, &this->error_);
	}

	Napi::Value WaitForFrames(const CallbackInfo& info) {
		auto frameset = ObjectWrap<RSFrameSet>::Unwrap(info[0].ToObject());
		if (!frameset) return Boolean::New(info.Env(), false);

		auto timeout	  = info[1].ToNumber().Int32Value();
		rs2_frame* frames = GetNativeResult<
		  rs2_frame*>(rs2_pipeline_wait_for_frames, &this->error_, this->pipeline_, timeout, &this->error_);
		if (!frames) return Boolean::New(info.Env(), false);

		frameset->Replace(frames);
		return Boolean::New(info.Env(), true);
	}

	Napi::Value PollForFrames(const CallbackInfo& info) {
		auto frameset = ObjectWrap<RSFrameSet>::Unwrap(info[0].ToObject());
		if (!frameset) return Boolean::New(info.Env(), false);

		rs2_frame* frames = nullptr;
		auto res
		  = GetNativeResult<int>(rs2_pipeline_poll_for_frames, &this->error_, this->pipeline_, &frames, &this->error_);
		if (!res) return Boolean::New(info.Env(), false);

		frameset->Replace(frames);
		return Boolean::New(info.Env(), true);
	}

	Napi::Value GetActiveProfile(const CallbackInfo& info) {
		rs2_pipeline_profile* prof = GetNativeResult<
		  rs2_pipeline_profile*>(rs2_pipeline_get_active_profile, &this->error_, this->pipeline_, &this->error_);
		if (!prof) return info.Env().Undefined();

		return RSPipelineProfile::NewInstance(info.Env(), prof);
	}

  private:
	static FunctionReference constructor;

	rs2_pipeline* pipeline_;
	rs2_error* error_;
};

Napi::FunctionReference RSPipeline::constructor;

#endif
