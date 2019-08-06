#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <napi.h>
#include <librealsense2/hpp/rs_types.hpp>
#include "device.cc"

using namespace Napi;

class RSDeviceList : public ObjectWrap<RSDeviceList> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSDeviceList",
		  {
			InstanceMethod("destroy", &RSDeviceList::Destroy),
			InstanceMethod("contains", &RSDeviceList::Contains),
			InstanceMethod("length", &RSDeviceList::Length),
			InstanceMethod("getDevice", &RSDeviceList::GetDevice),
		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSDeviceList", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_device_list* list) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		auto unwrapped = ObjectWrap<RSDeviceList>::Unwrap(instance);
		unwrapped->list_ = list;

		return scope.Escape(napi_value(instance)).ToObject();
	}

	RSDeviceList(const CallbackInfo& info)
	  : ObjectWrap<RSDeviceList>(info)
	  , error_(nullptr)
	  , list_(nullptr) {
	}

	~RSDeviceList() {
		DestroyMe();
	}

  private:
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
		if (this->error_) throw Napi::Error::New(info.Env(), "Error trying to check if device list contains device");;

		return Boolean::New(info.Env(), contains);
	}

	Napi::Value Length(const CallbackInfo& info) {
		auto cnt = GetNativeResult<int>(rs2_get_device_count, &this->error_, this->list_, &this->error_);
		return Number::New(info.Env(), cnt);
	}

	Napi::Value GetDevice(const CallbackInfo& info) {
		auto index = info[0].As<Number>().ToNumber().Uint32Value();
		auto dev = GetNativeResult<rs2_device*>(rs2_create_device, &this->error_, this->list_, index, &this->error_);

		return RSDevice::NewInstance(info.Env(), dev);
	}

	static FunctionReference constructor;
	rs2_error* error_;
	rs2_device_list* list_;
};

Napi::FunctionReference RSDeviceList::constructor;

#endif
