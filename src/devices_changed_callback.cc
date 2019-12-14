#ifndef DEVICES_CHANGED_CALLBACK_H
#define DEVICES_CHANGED_CALLBACK_H

#include "context.cc"
#include "napi-thread-safe-callback.hpp"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class DevicesChangedCallback : public rs2_devices_changed_callback {
  public:
	std::shared_ptr<ThreadSafeCallback> fn_;
	RSContext* ctx_;

	DevicesChangedCallback(RSContext* context, std::shared_ptr<ThreadSafeCallback> fn)
	  : ctx_(context)
	  , fn_(fn) {
		std::cerr << __FILE__ << ":" << __LINE__ << "\tDevicesChangedCallback::DevicesChangedCallback" << std::endl;
	}

	virtual void on_devices_changed(rs2_device_list* removed, rs2_device_list* added) {
		std::cerr << __FILE__ << ":" << __LINE__ << "\tDevicesChangedCallback::on_devices_changed" << this->fn_
				  << std::endl;

		this->fn_->call([removed, added](Napi::Env env, std::vector<napi_value>& args) {
			Value rmList;

			if (removed)
				rmList = RSDeviceList::NewInstance(env, removed);
			else
				rmList = env.Undefined();

			Value addList;

			if (added)
				addList = RSDeviceList::NewInstance(env, added);
			else
				addList = env.Undefined();

			args = { rmList, addList };
		});
	}

	virtual void release() {
		std::cerr << __FILE__ << ":" << __LINE__ << "\tDevicesChangedCallback::Release" << std::endl;
		delete this;
	}

	virtual ~DevicesChangedCallback() {
		std::cerr << __FILE__ << ":" << __LINE__ << "\tDevicesChangedCallback::~DevicesChangedCallback" << std::endl;
	}
};

void RSContext::RegisterDevicesChangedCallbackMethod(std::shared_ptr<ThreadSafeCallback> callback) {
	std::cerr << "RSContext::RegisterDevicesChangedCallbackMethod" << std::endl;
	rs2_set_devices_changed_callback_cpp(this->ctx_, new DevicesChangedCallback(this, callback), &this->error_);
}

#endif
