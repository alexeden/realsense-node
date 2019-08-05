#ifndef PIPELINE_H
#define PIPELINE_H

#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSPipeline : public Nan::ObjectWrap {
  public:
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSPipeline").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "start", Start);
		Nan::SetPrototypeMethod(tpl, "startWithConfig", StartWithConfig);
		Nan::SetPrototypeMethod(tpl, "stop", Stop);
		Nan::SetPrototypeMethod(tpl, "waitForFrames", WaitForFrames);
		Nan::SetPrototypeMethod(tpl, "pollForFrames", PollForFrames);
		Nan::SetPrototypeMethod(tpl, "getActiveProfile", GetActiveProfile);
		Nan::SetPrototypeMethod(tpl, "create", Create);
		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);

		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSPipeline").ToLocalChecked(), tpl->GetFunction());
	}

	static v8::Local<v8::Object> NewInstance() {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor_);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
		return scope.Escape(instance);
	}

  private:
	friend class RSConfig;

	RSPipeline()
	  : pipeline_(nullptr)
	  , error_(nullptr) {
	}

	~RSPipeline() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (pipeline_) rs2_delete_pipeline(pipeline_);
		pipeline_ = nullptr;
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		if (me) me->DestroyMe();
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (info.IsConstructCall()) {
			RSPipeline* obj = new RSPipeline();
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	static NAN_METHOD(Create) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me	= Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		auto rsctx = Nan::ObjectWrap::Unwrap<RSContext>(info[0]->ToObject());
		if (!me || !rsctx) return;

		me->pipeline_ = GetNativeResult<rs2_pipeline*>(rs2_create_pipeline, &me->error_, rsctx->ctx_, &me->error_);
	}

	static NAN_METHOD(StartWithConfig) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		if (!me || !me->pipeline_) return;

		RSConfig* config		   = Nan::ObjectWrap::Unwrap<RSConfig>(info[0]->ToObject());
		rs2_pipeline_profile* prof = GetNativeResult<
		  rs2_pipeline_profile*>(rs2_pipeline_start_with_config, &me->error_, me->pipeline_, config->config_, &me->error_);
		if (!prof) return;

		info.GetReturnValue().Set(RSPipelineProfile::NewInstance(prof));
	}

	static NAN_METHOD(Start) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		if (!me || !me->pipeline_) return;

		rs2_pipeline_profile* prof
		  = GetNativeResult<rs2_pipeline_profile*>(rs2_pipeline_start, &me->error_, me->pipeline_, &me->error_);
		if (!prof) return;

		info.GetReturnValue().Set(RSPipelineProfile::NewInstance(prof));
	}

	static NAN_METHOD(Stop) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		if (!me || !me->pipeline_) return;

		CallNativeFunc(rs2_pipeline_stop, &me->error_, me->pipeline_, &me->error_);
	}

	static NAN_METHOD(WaitForFrames) {
		info.GetReturnValue().Set(Nan::False());
		auto me		  = Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		auto frameset = Nan::ObjectWrap::Unwrap<RSFrameSet>(info[0]->ToObject());
		if (!me || !frameset) return;

		auto timeout = info[1]->IntegerValue();
		rs2_frame* frames
		  = GetNativeResult<rs2_frame*>(rs2_pipeline_wait_for_frames, &me->error_, me->pipeline_, timeout, &me->error_);
		if (!frames) return;

		frameset->Replace(frames);
		info.GetReturnValue().Set(Nan::True());
	}

	static NAN_METHOD(PollForFrames) {
		info.GetReturnValue().Set(Nan::False());
		auto me		  = Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		auto frameset = Nan::ObjectWrap::Unwrap<RSFrameSet>(info[0]->ToObject());
		if (!me || !frameset) return;

		rs2_frame* frames = nullptr;
		auto res = GetNativeResult<int>(rs2_pipeline_poll_for_frames, &me->error_, me->pipeline_, &frames, &me->error_);
		if (!res) return;

		frameset->Replace(frames);
		info.GetReturnValue().Set(Nan::True());
	}

	static NAN_METHOD(GetActiveProfile) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSPipeline>(info.Holder());
		if (!me) return;

		rs2_pipeline_profile* prof = GetNativeResult<
		  rs2_pipeline_profile*>(rs2_pipeline_get_active_profile, &me->error_, me->pipeline_, &me->error_);
		if (!prof) return;

		info.GetReturnValue().Set(RSPipelineProfile::NewInstance(prof));
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;

	rs2_pipeline* pipeline_;
	rs2_error* error_;
};

Nan::Persistent<v8::Function> RSPipeline::constructor_;

#endif
