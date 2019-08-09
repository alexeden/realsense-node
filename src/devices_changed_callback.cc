#ifndef DEVICES_CHANGED_CALLBACK_H
#define DEVICES_CHANGED_CALLBACK_H

#include "context.cc"
#include "main_thread_callback.cc"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class DevicesChangedCallbackInfo : public MainThreadCallbackInfo {
  public:
	DevicesChangedCallbackInfo(rs2_device_list* r, rs2_device_list* a, RSContext* ctx, Napi::Env env)
	  : removed_(r)
	  , added_(a)
	  , ctx_(ctx)
	  , env_(env) {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::DevicesChangedCallbackInfo"
				  << std::endl;
	}

	~DevicesChangedCallbackInfo() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::~DevicesChangedCallbackInfo"
				  << std::endl;
		if (!consumed_) Release();
	}
	void Run() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::Run" << std::endl;
		SetConsumed();
		std::cerr << __FILE__ << ":" << __LINE__ << " Run" << std::endl;
		auto env = this->env_;
		// env.
		std::cerr << __FILE__ << ":" << __LINE__ << env << std::endl;
		// EscapableHandleScope scope(this->env_);
		// std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
		// // auto handle = scope.Escape(napi_value(this->ctx_)).ToObject();
		// std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

		// // AsyncContext async_ctx(this->env_, "devices_changed_callback_info", scope);
		// std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
		// auto ctx = scope.Escape(napi_value(instance)).ToObject();
		// auto ctx = ObjectWrap<RSContext>::Unwrap(ctx_);
		// HandleScope scope;
		// Value rmList;
		// if (removed_)
		// 	rmList = RSDeviceList::NewInstance(env, removed_);
		// else
		// 	rmList = env.Undefined();

		// Value addList;
		// if (added_)
		// 	addList = RSDeviceList::NewInstance(env, added_);
		// else
		// 	addList = env.Undefined();

		// this->ctx_->Value().Get(this->ctx_->device_changed_callback_name_)
		// this->ctx_->device_changed_callback_.MakeCallback(Napi::Object::New(this->ctx_->Env()), {});
		// this->ctx_->device_changed_callback_.Call({});
		// this->ctx_->device_changed_callback_.MakeCallback(Napi::Object::New(this->env_), {}, async_ctx);
	}
	void Release() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::Release" << std::endl;

		if (removed_) {
			rs2_delete_device_list(removed_);
			removed_ = nullptr;
		}

		if (added_) {
			rs2_delete_device_list(added_);
			added_ = nullptr;
		}
	}

  private:
	rs2_device_list* removed_;
	rs2_device_list* added_;
	// AsyncContext async_ctx_;
	RSContext* ctx_;
	Napi::Env env_;
};

class DevicesChangedCallback : public rs2_devices_changed_callback {
  public:
	DevicesChangedCallback(RSContext* context, Env env)
	  : ctx_(context)
	  , env_(env) {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallback::DevicesChangedCallback" << std::endl;
	}
	virtual void on_devices_changed(rs2_device_list* removed, rs2_device_list* added) {
		std::cerr << __FILE__ << ":" << __LINE__ << "DevicesChangedCallback::on_devices_changed" << std::endl;
		EscapableHandleScope scope(this->env_);
		// auto result = this->ctx_->device_changed_callback_.Call(this, {});
		scope.Escape(this->ctx_->device_changed_callback_.Call({}));

		// MainThreadCallback::NotifyMainThread(new DevicesChangedCallbackInfo(removed, added, ctx_, env_));
	}

	virtual void release() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallback::Release" << std::endl;
		// delete this;
	}

	virtual ~DevicesChangedCallback() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallback::~DevicesChangedCallback" << std::endl;
	}
	RSContext* ctx_;
	Env env_;
};

void RSContext::RegisterDevicesChangedCallbackMethod(Napi::Env env) {
	std::cerr << "RSContext::RegisterDevicesChangedCallbackMethod" << std::endl;
	rs2_set_devices_changed_callback_cpp(this->ctx_, new DevicesChangedCallback(this, env), &this->error_);
	// CallNativeFunc(rs2_set_devices_changed_callback_cpp, &error_, this->ctx_, new DevicesChangedCallback(this, env), &error_);
}

#endif
