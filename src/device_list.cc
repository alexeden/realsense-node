#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <napi.h>
// #include <librealsense2/h/rs_internal.h>
#include <librealsense2/hpp/rs_types.hpp>
// #include <librealsense2/rs.h>
#include "device.cc"


using namespace Napi;

class RSDeviceList : ObjectWrap<RSDeviceList> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSDeviceList",
		  {
			InstanceMethod("destroy", &RSDeviceList::Destroy),
			InstanceMethod("contains", &RSDeviceList::Contains),
			InstanceMethod("size", &RSDeviceList::Size),
			InstanceMethod("getDevice", &RSDeviceList::GetDevice),
		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSDeviceList", func);

		return exports;
	}

	static Object NewInstance(rs2_device_list* list) {
		Nan::EscapableHandleScope scope;
		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
		Object instance				   = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
		auto me						   = ObjectWrap<RSDeviceList>::Unwrap(instance);
		me->list_					   = list;
		return scope.Escape(instance);
	}

  private:
	RSDeviceList(const CallbackInfo& info)
	  : ObjectWrap<RSDeviceList>(info)
	  , error_(nullptr)
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

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	Napi::Value Contains(const CallbackInfo& info) {
		auto dev = ObjectWrap<RSDevice>::Unwrap(info[0].As<Object>());
		bool contains = GetNativeResult<int>(rs2_device_list_contains, &this->error_, this->list_, dev->dev_, &this->error_);
		if (this->error_) return;

		return Boolean::New(info.Env(), contains);
	}

	Napi::Value Size(const CallbackInfo& info) {
		auto cnt = GetNativeResult<int>(rs2_get_device_count, &this->error_, this->list_, &this->error_);
		return Number::New(info.Env(), cnt);
	}

	Napi::Value GetDevice(const CallbackInfo& info) {
		auto index = info[0].As<Number>().ToNumber().Uint32Value();

		auto dev = GetNativeResult<rs2_device*>(rs2_create_device, &this->error_, this->list_, index, &this->error_);

		return RSDevice::NewInstance(dev);
	}

  private:
	static FunctionReference constructor;
	rs2_error* error_;
	rs2_device_list* list_;
};

Napi::FunctionReference RSDeviceList::constructor;

#endif
