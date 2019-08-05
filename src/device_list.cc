#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <napi.h>

using namespace Napi;

class RSDeviceList : public Nan::ObjectWrap {
  public:
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSDeviceList").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "contains", Contains);
		Nan::SetPrototypeMethod(tpl, "size", Size);
		Nan::SetPrototypeMethod(tpl, "getDevice", GetDevice);

		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSDeviceList").ToLocalChecked(), tpl->GetFunction());
	}

	static v8::Local<v8::Object> NewInstance(rs2_device_list* list) {
		Nan::EscapableHandleScope scope;
		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor_);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
		auto me						   = Nan::ObjectWrap::Unwrap<RSDeviceList>(instance);
		me->list_					   = list;
		return scope.Escape(instance);
	}

  private:
	RSDeviceList()
	  : error_(nullptr)
	  , list_(nullptr) {
	}

	~RSDeviceList() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (list_) rs2_delete_device_list(list_);
		list_ = nullptr;
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (info.IsConstructCall()) {
			RSDeviceList* obj = new RSDeviceList();
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSDeviceList>(info.Holder());
		if (me) me->DestroyMe();

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(Contains) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me  = Nan::ObjectWrap::Unwrap<RSDeviceList>(info.Holder());
		auto dev = Nan::ObjectWrap::Unwrap<RSDevice>(info[0]->ToObject());
		if (!me && dev) return;

		bool contains = GetNativeResult<int>(rs2_device_list_contains, &me->error_, me->list_, dev->dev_, &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(contains));
	}

	static NAN_METHOD(Size) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDeviceList>(info.Holder());
		if (!me) return;

		auto cnt = GetNativeResult<int>(rs2_get_device_count, &me->error_, me->list_, &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(cnt));
	}

	static NAN_METHOD(GetDevice) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me	= Nan::ObjectWrap::Unwrap<RSDeviceList>(info.Holder());
		auto index = info[0]->IntegerValue();
		if (!me) return;

		auto dev = GetNativeResult<rs2_device*>(rs2_create_device, &me->error_, me->list_, index, &me->error_);
		if (!dev) return;

		info.GetReturnValue().Set(RSDevice::NewInstance(dev));
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;
	rs2_error* error_;
	rs2_device_list* list_;
};

Nan::Persistent<v8::Function> RSDeviceList::constructor_;

#endif
