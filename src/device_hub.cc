#ifndef DEVICE_HUB_H
#define DEVICE_HUB_H

#include "device.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSDeviceHub : public ObjectWrap<RSDeviceHub> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSDeviceHub",
		  {
			InstanceMethod("waitForDevice", &RSDeviceHub::WaitForDevice),
			InstanceMethod("isConnected", &RSDeviceHub::IsConnected),
			InstanceMethod("destroy", &RSDeviceHub::Destroy),
		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSDeviceHub", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		return scope.Escape(napi_value(instance)).ToObject();
	}

	RSDeviceHub(const CallbackInfo& info)
	  : ObjectWrap<RSDeviceHub>(info)
	  , hub_(nullptr)
	  , ctx_(nullptr)
	  , error_(nullptr) {
	}

	~RSDeviceHub() {
		DestroyMe();
	}

  private:
	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;

		if (hub_) rs2_delete_device_hub(hub_);
		hub_ = nullptr;
		ctx_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (info.IsConstructCall()) {
			RSDeviceHub* obj = new RSDeviceHub();
			RSContext* ctx   = Nan::ObjectWrap::Unwrap<RSContext>(info[0]->ToObject());
			obj->ctx_		 = ctx->ctx_;
			obj->hub_ = GetNativeResult<rs2_device_hub*>(rs2_create_device_hub, &obj->error_, obj->ctx_, &obj->error_);
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	Napi::Value WaitForDevice(const CallbackInfo& info) {
		auto dev
		  = GetNativeResult<rs2_device*>(rs2_device_hub_wait_for_device, &this->error_, this->hub_, &this->error_);
		if (!dev) return info.Env().Undefined();

		return RSDevice::NewInstance(info.Env(), dev);
	}

	Napi::Value IsConnected(const CallbackInfo& info) {
		auto dev = ObjectWrap<RSDevice>::Unwrap(info[0].ToObject());
		if (!dev) return;

		auto res = GetNativeResult<
		  int>(rs2_device_hub_is_device_connected, &this->error_, this->hub_, dev->dev_, &this->error_);
		if (this->error_) return;

		return Boolean::New(res ? true : false);
	}

  private:
	static FunctionReference constructor;

	rs2_device_hub* hub_;
	rs2_context* ctx_;
	rs2_error* error_;
};

Napi::FunctionReference RSDeviceHub::constructor;

#endif
