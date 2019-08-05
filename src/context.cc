#ifndef CONTEXT_H
#define CONTEXT_H

#include <napi.h>

class RSContext : public Nan::ObjectWrap {
  public:
	enum ContextType {
		kNormal = 0,
		kRecording,
		kPlayback,
	};
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSContext").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "create", Create);
		Nan::SetPrototypeMethod(tpl, "queryDevices", QueryDevices);
		Nan::SetPrototypeMethod(tpl, "setDevicesChangedCallback", SetDevicesChangedCallback);
		Nan::SetPrototypeMethod(tpl, "loadDeviceFile", LoadDeviceFile);
		Nan::SetPrototypeMethod(tpl, "unloadDeviceFile", UnloadDeviceFile);
		Nan::SetPrototypeMethod(tpl, "createDeviceFromSensor", CreateDeviceFromSensor);

		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSContext").ToLocalChecked(), tpl->GetFunction());
	}

	static v8::Local<v8::Object> NewInstance(rs2_context* ctx_ptr = nullptr) {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor_);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();

		// If ctx_ptr is provided, no need to call create.
		if (ctx_ptr) {
			auto me  = Nan::ObjectWrap::Unwrap<RSContext>(instance);
			me->ctx_ = ctx_ptr;
		}
		return scope.Escape(instance);
	}

  private:
	explicit RSContext(ContextType type = kNormal)
	  : ctx_(nullptr)
	  , error_(nullptr)
	  , type_(type)
	  , mode_(RS2_RECORDING_MODE_BLANK_FRAMES) {
	}

	~RSContext() {
		DestroyMe();
	}

	void RegisterDevicesChangedCallbackMethod();

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (ctx_) rs2_delete_context(ctx_);
		ctx_ = nullptr;
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (!info.IsConstructCall()) return;

		ContextType type = kNormal;
		if (info.Length()) {
			v8::String::Utf8Value type_str(info[0]->ToString());
			std::string std_type_str(*type_str);
			if (!std_type_str.compare("recording"))
				type = kRecording;
			else if (!std_type_str.compare("playback"))
				type = kPlayback;
		}
		RSContext* obj = new RSContext(type);
		if (type == kRecording || type == kPlayback) {
			v8::String::Utf8Value file(info[1]->ToString());
			v8::String::Utf8Value section(info[2]->ToString());
			obj->file_name_ = std::string(*file);
			obj->section_   = std::string(*section);
		}
		if (type == kRecording) obj->mode_ = static_cast<rs2_recording_mode>(info[3]->IntegerValue());
		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}

	static NAN_METHOD(Create) {
		MainThreadCallback::Init();
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSContext>(info.Holder());
		if (!me) return;

		switch (me->type_) {
			case kRecording:
				me->ctx_ = GetNativeResult<rs2_context*>(
				  rs2_create_recording_context,
				  &me->error_,
				  RS2_API_VERSION,
				  me->file_name_.c_str(),
				  me->section_.c_str(),
				  me->mode_,
				  &me->error_);
				break;
			case kPlayback:
				me->ctx_ = GetNativeResult<rs2_context*>(
				  rs2_create_mock_context,
				  &me->error_,
				  RS2_API_VERSION,
				  me->file_name_.c_str(),
				  me->section_.c_str(),
				  &me->error_);
				break;
			default:
				me->ctx_ = GetNativeResult<rs2_context*>(rs2_create_context, &me->error_, RS2_API_VERSION, &me->error_);
				break;
		}
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSContext>(info.Holder());
		if (me) { me->DestroyMe(); }
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(SetDevicesChangedCallback) {
		auto me = Nan::ObjectWrap::Unwrap<RSContext>(info.Holder());
		if (me) {
			v8::String::Utf8Value value(info[0]->ToString());
			me->device_changed_callback_name_ = std::string(*value);
			me->RegisterDevicesChangedCallbackMethod();
		}
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(LoadDeviceFile) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSContext>(info.Holder());
		if (!me) return;

		auto device_file = info[0]->ToString();
		v8::String::Utf8Value value(device_file);
		auto dev = GetNativeResult<rs2_device*>(rs2_context_add_device, &me->error_, me->ctx_, *value, &me->error_);
		if (!dev) return;

		auto jsobj = RSDevice::NewInstance(dev, RSDevice::kPlaybackDevice);
		info.GetReturnValue().Set(jsobj);
	}

	static NAN_METHOD(UnloadDeviceFile) {
		auto me = Nan::ObjectWrap::Unwrap<RSContext>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto device_file = info[0]->ToString();
		v8::String::Utf8Value value(device_file);
		CallNativeFunc(rs2_context_remove_device, &me->error_, me->ctx_, *value, &me->error_);
	}

	static NAN_METHOD(CreateDeviceFromSensor) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto sensor = Nan::ObjectWrap::Unwrap<RSSensor>(info[0]->ToObject());
		if (!sensor) return;

		rs2_error* error = nullptr;
		auto dev		 = GetNativeResult<rs2_device*>(rs2_create_device_from_sensor, &error, sensor->sensor_, &error);
		if (!dev) return;

		auto jsobj = RSDevice::NewInstance(dev);
		info.GetReturnValue().Set(jsobj);
	}

	static NAN_METHOD(QueryDevices) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSContext>(info.Holder());
		if (!me) return;

		auto dev_list = GetNativeResult<rs2_device_list*>(rs2_query_devices, &me->error_, me->ctx_, &me->error_);
		if (!dev_list) return;

		auto jsobj = RSDeviceList::NewInstance(dev_list);
		info.GetReturnValue().Set(jsobj);
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;

	rs2_context* ctx_;
	rs2_error* error_;
	std::string device_changed_callback_name_;
	ContextType type_;
	std::string file_name_;
	std::string section_;
	rs2_recording_mode mode_;
	friend class DevicesChangedCallbackInfo;
	friend class RSPipeline;
	friend class RSDeviceHub;
};

Nan::Persistent<v8::Function> RSContext::constructor_;

#endif
