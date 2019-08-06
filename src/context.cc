#ifndef CONTEXT_H
#define CONTEXT_H

#include "main_thread_callback.cc"
// #include "sensor.cc"
#include "device_list.cc"
#include "utils.cc"
#include <librealsense2/h/rs_internal.h>
#include <librealsense2/hpp/rs_types.hpp>
#include <librealsense2/rs.h>
#include <napi.h>

using namespace Napi;

class RSContext : public ObjectWrap<RSContext> {
  public:
	enum ContextType {
		kNormal = 0,
		kRecording,
		kPlayback,
	};

	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSContext",
		  {
			InstanceMethod("destroy", &RSContext::Destroy),
			InstanceMethod("create", &RSContext::Create),
			InstanceMethod("queryDevices", &RSContext::QueryDevices),
			InstanceMethod("setDevicesChangedCallback", &RSContext::SetDevicesChangedCallback),
			// InstanceMethod("loadDeviceFile", &RSContext::LoadDeviceFile),
			// InstanceMethod("unloadDeviceFile", &RSContext::UnloadDeviceFile),
			// InstanceMethod("createDeviceFromSensor", &RSContext::CreateDeviceFromSensor),

		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSDevice", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_context* ctx_ptr = nullptr) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		// If ctx_ptr is provided, no need to call create.
		// if (ctx_ptr) {
		// 	this->ctx_ = ctx_ptr;
		// }
		return scope.Escape(napi_value(instance)).ToObject();
	}

	RSContext(const CallbackInfo& info)
	  // RSContext(ContextType type = kNormal)
	  : ObjectWrap<RSContext>(info)
	  , ctx_(nullptr)
	  , error_(nullptr)
	  , mode_(RS2_RECORDING_MODE_BLANK_FRAMES) {
		this->type_ = info[0].IsNumber() ? info[0].As<ContextType>() : kNormal;

		ContextType type = kNormal;
		if (info.Length()) {
			std::string std_type_str = info[0].As<String>().ToString();
			if (!std_type_str.compare("recording"))
				this->type_ = kRecording;
			else if (!std_type_str.compare("playback"))
				this->type_ = kPlayback;
		}
		if (type == kRecording || type == kPlayback) {
			std::string file	= info[1].As<String>().ToString();
			std::string section = info[2].As<String>().ToString();
			this->file_name_	= file;
			this->section_		= section;
		}
		if (type == kRecording)
			this->mode_ = static_cast<rs2_recording_mode>(info[3].As<Number>().ToNumber().Uint32Value());
	}

	~RSContext() {
		DestroyMe();
	}

  private:
	void RegisterDevicesChangedCallbackMethod();

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (ctx_) rs2_delete_context(ctx_);
		ctx_ = nullptr;
	}

	Napi::Value Create(const CallbackInfo& info) {
		MainThreadCallback::Init();

		switch (this->type_) {
			case kRecording:
				this->ctx_ = GetNativeResult<rs2_context*>(
				  rs2_create_recording_context,
				  &this->error_,
				  RS2_API_VERSION,
				  this->file_name_.c_str(),
				  this->section_.c_str(),
				  this->mode_,
				  &this->error_);
				break;
			case kPlayback:
				this->ctx_ = GetNativeResult<rs2_context*>(
				  rs2_create_mock_context,
				  &this->error_,
				  RS2_API_VERSION,
				  this->file_name_.c_str(),
				  this->section_.c_str(),
				  &this->error_);
				break;
			default:
				this->ctx_
				  = GetNativeResult<rs2_context*>(rs2_create_context, &this->error_, RS2_API_VERSION, &this->error_);
				break;
		}
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	Napi::Value SetDevicesChangedCallback(const CallbackInfo& info) {
		this->device_changed_callback_name_ = info[0].As<String>().ToString();
		this->RegisterDevicesChangedCallbackMethod();

		return info.This();
	}

	// Napi::Value LoadDeviceFile(const CallbackInfo& info) {
	// 	std::string device_file = info[0].As<String>().ToString();
	// 	auto dev = GetNativeResult<rs2_device*>(rs2_context_add_device, &this->error_, this->ctx_, device_file, &this->error_);
	// 	if (!dev) return;

	// 	auto jsobj = RSDevice::NewInstance(dev, RSDevice::kPlaybackDevice);
	// 	return jsobj;
	// }

	// Napi::Value UnloadDeviceFile(const CallbackInfo& info) {
	// 	std::string device_file = info[0].As<String>().ToString();
	// 	CallNativeFunc(rs2_context_remove_device, &this->error_, this->ctx_, device_file, &this->error_);
	// }

	// Napi::Value CreateDeviceFromSensor(const CallbackInfo& info) {
	// 	auto sensor = ObjectWrap<RSSensor>::Unwrap(info[0].As<Object>());

	// 	rs2_error* error = nullptr;
	// 	auto dev		 = GetNativeResult<rs2_device*>(rs2_create_device_from_sensor, &error, sensor->sensor_, &error);
	// 	if (!dev) return;

	// 	return RSDevice::NewInstance(dev);
	// }

	Napi::Value QueryDevices(const CallbackInfo& info) {
		auto dev_list = GetNativeResult<rs2_device_list*>(rs2_query_devices, &this->error_, this->ctx_, &this->error_);

		return RSDeviceList::NewInstance(info.Env(), dev_list);
	}

  private:
	static FunctionReference constructor;

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

Napi::FunctionReference RSContext::constructor;

#endif
