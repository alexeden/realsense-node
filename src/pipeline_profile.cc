#ifndef PIPELINE_PROFILE_H
#define PIPELINE_PROFILE_H

#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSPipelineProfile : public Nan::ObjectWrap {
  public:
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSPipelineProfile").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "getStreams", GetStreams);
		Nan::SetPrototypeMethod(tpl, "getDevice", GetDevice);
		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSPipelineProfile").ToLocalChecked(), tpl->GetFunction());
	}

	static v8::Local<v8::Object> NewInstance(rs2_pipeline_profile* profile) {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor_);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();

		auto me				  = Nan::ObjectWrap::Unwrap<RSPipelineProfile>(instance);
		me->pipeline_profile_ = profile;
		return scope.Escape(instance);
	}

  private:
	RSPipelineProfile()
	  : pipeline_profile_(nullptr)
	  , error_(nullptr) {
	}

	~RSPipelineProfile() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;

		if (pipeline_profile_) rs2_delete_pipeline_profile(pipeline_profile_);
		pipeline_profile_ = nullptr;
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSPipelineProfile>(info.Holder());
		if (me) me->DestroyMe();
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (info.IsConstructCall()) {
			RSPipelineProfile* obj = new RSPipelineProfile();
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	static NAN_METHOD(GetStreams) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSPipelineProfile>(info.Holder());
		if (!me) return;

		rs2_stream_profile_list* list = GetNativeResult<
		  rs2_stream_profile_list*>(rs2_pipeline_profile_get_streams, &me->error_, me->pipeline_profile_, &me->error_);
		if (!list) return;

		int32_t size = GetNativeResult<int32_t>(rs2_get_stream_profiles_count, &me->error_, list, &me->error_);
		if (me->error_) return;

		v8::Local<v8::Array> array = Nan::New<v8::Array>(size);
		for (int32_t i = 0; i < size; i++) {
			rs2_stream_profile* profile = const_cast<rs2_stream_profile*>(
			  GetNativeResult<const rs2_stream_profile*>(rs2_get_stream_profile, &me->error_, list, i, &me->error_));
			array->Set(i, RSStreamProfile::NewInstance(profile));
		}
		info.GetReturnValue().Set(array);
	}

	static NAN_METHOD(GetDevice) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSPipelineProfile>(info.Holder());
		if (!me) return;

		rs2_device* dev = GetNativeResult<
		  rs2_device*>(rs2_pipeline_profile_get_device, &me->error_, me->pipeline_profile_, &me->error_);
		if (!dev) return;

		info.GetReturnValue().Set(RSDevice::NewInstance(dev));
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;

	rs2_pipeline_profile* pipeline_profile_;
	rs2_error* error_;
};

Nan::Persistent<v8::Function> RSPipelineProfile::constructor_;

#endif
