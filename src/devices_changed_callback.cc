#ifndef DEVICES_CHANGED_CALLBACK_H
#define DEVICES_CHANGED_CALLBACK_H

#include "./context.cc"
#include "./main_thread_callback.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

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
		Nan::HandleScope scope;
		v8::Local<v8::Value> rmlist;
		v8::Local<v8::Value> addlist;
		if (removed_)
			rmlist = RSDeviceList::NewInstance(removed_);
		else
			rmlist = Nan::Undefined();

		if (added_)
			addlist = RSDeviceList::NewInstance(added_);
		else
			addlist = Nan::Undefined();

		v8::Local<v8::Value> args[2] = { rmlist, addlist };
		Nan::MakeCallback(ctx_->handle(), ctx_->device_changed_callback_name_.c_str(), 2, args);
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
	CallNativeFunc(rs2_set_devices_changed_callback_cpp, &error_, ctx_, new DevicesChangedCallback(this), &error_);
}

#endif
