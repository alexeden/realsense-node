#ifndef DEVICES_CHANGED_CALLBACK_H
#define DEVICES_CHANGED_CALLBACK_H

#include <iostream>
#include <napi.h>
#include <librealsense2/hpp/rs_types.hpp>
#include "context.cc"
#include "main_thread_callback.cc"

using namespace Napi;

class DevicesChangedCallbackInfo : public MainThreadCallbackInfo {
  public:
	DevicesChangedCallbackInfo(rs2_device_list* r, rs2_device_list* a, RSContext* ctx)
	  : removed_(r)
	  , added_(a)
	  , ctx_(ctx) {
	}
	virtual ~DevicesChangedCallbackInfo() {
		if (!consumed_) Release();
	}
	virtual void Run() {
		SetConsumed();
		// auto ctx = scope.Escape(napi_value(instance)).ToObject();
		auto env = this->ctx_->Env();
		// auto ctx = ObjectWrap<RSContext>::Unwrap(ctx_);
		// HandleScope scope;
		Value rmlist = removed_
			? RSDeviceList::NewInstance(env, removed_)
			: env.Undefined();

		Value addlist = added_
			? RSDeviceList::NewInstance(env, added_)
			: env.Undefined();

		// this->ctx_->Value().Get(this->ctx_->device_changed_callback_name_)
		this->ctx_->device_changed_callback_.Call({ rmlist, addlist });
		// Function::MakeCallback(this->ctx_, args);
	}
	virtual void Release() {
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
	RSContext* ctx_;
};

class DevicesChangedCallback : public rs2_devices_changed_callback {
  public:
	explicit DevicesChangedCallback(RSContext* context)
	  : ctx_(context) {
	}
	virtual void on_devices_changed(rs2_device_list* removed, rs2_device_list* added) {
		MainThreadCallback::NotifyMainThread(new DevicesChangedCallbackInfo(removed, added, ctx_));
	}

	virtual void release() {
		delete this;
	}

	virtual ~DevicesChangedCallback() {
	}
	RSContext* ctx_;
};

void RSContext::RegisterDevicesChangedCallbackMethod() {
	std::cerr << "RSContext::RegisterDevicesChangedCallbackMethod" << std::endl;
	CallNativeFunc(rs2_set_devices_changed_callback_cpp, &error_, ctx_, new DevicesChangedCallback(this), &error_);
}

#endif
