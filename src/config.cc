#ifndef CONFIG_H
#define CONFIG_H

#include "utils.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSConfig : public ObjectWrap<RSConfig> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSConfig",
		  {
			InstanceMethod("destroy", &RSConfig::Destroy),
			InstanceMethod("disableAllStreams", &RSConfig::DisableAllStreams),
			InstanceMethod("disableStream", &RSConfig::DisableStream),
			InstanceMethod("enableAllStreams", &RSConfig::EnableAllStreams),
			InstanceMethod("enableDevice", &RSConfig::EnableDevice),
			InstanceMethod("enableDeviceFromFile", &RSConfig::EnableDeviceFromFile),
			InstanceMethod("enableDeviceFromFileRepeatOption", &RSConfig::EnableDeviceFromFileRepeatOption),
			InstanceMethod("enableRecordToFile", &RSConfig::EnableRecordToFile),
			InstanceMethod("enableStream", &RSConfig::EnableStream),
		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSConfig", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		return scope.Escape(napi_value(instance)).ToObject();
	}

	RSConfig(const CallbackInfo& info)
	  : ObjectWrap<RSConfig>(info)
	  , config_(nullptr)
	  , error_(nullptr) {
		this->config_ = rs2_create_config(&this->error_);
	}

	~RSConfig() {
		DestroyMe();
	}

  private:
	static FunctionReference constructor;
	friend class RSPipeline;

	rs2_config* config_;
	rs2_error* error_;

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (config_) rs2_delete_config(config_);
		config_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.This();
	}

	Napi::Value DisableAllStreams(const CallbackInfo& info) {
		CallNativeFunc(rs2_config_disable_all_streams, &this->error_, this->config_, &this->error_);
		return info.This();
	}

    Napi::Value DisableStream(const CallbackInfo& info) {
		auto stream = info[0].ToNumber().Int32Value();
		CallNativeFunc(rs2_config_disable_stream, &this->error_, this->config_, (rs2_stream) stream, &this->error_);
		return info.This();
	}

	Napi::Value EnableAllStreams(const CallbackInfo& info) {
		CallNativeFunc(rs2_config_enable_all_stream, &this->error_, this->config_, &this->error_);
		return info.This();
	}

	Napi::Value EnableDevice(const CallbackInfo& info) {
		auto device = std::string(info[0].ToString());
		CallNativeFunc(rs2_config_enable_device, &this->error_, this->config_, device.c_str(), &this->error_);
		return info.This();
	}

	Napi::Value EnableDeviceFromFile(const CallbackInfo& info) {
		auto device_file = std::string(info[0].ToString());
		CallNativeFunc(rs2_config_enable_device_from_file, &this->error_, this->config_, device_file.c_str(), &this->error_);
		return info.This();
	}

	Napi::Value EnableDeviceFromFileRepeatOption(const CallbackInfo& info) {
		auto device_file = std::string(info[0].ToString());
		auto repeat		 = info[1].ToBoolean();
		CallNativeFunc(
		  rs2_config_enable_device_from_file_repeat_option,
		  &this->error_,
		  this->config_,
		  device_file.c_str(),
		  repeat,
		  &this->error_);
		return info.This();
	}

	Napi::Value EnableRecordToFile(const CallbackInfo& info) {
		auto device_file = std::string(info[0].ToString());
		CallNativeFunc(rs2_config_enable_record_to_file, &this->error_, this->config_, device_file.c_str(), &this->error_);
		return info.This();
	}

	// TODO(halton): added all the overloads
	Napi::Value EnableStream(const CallbackInfo& info) {
		auto stream	   = info[0].ToNumber().Int32Value();
		auto index	   = info[1].ToNumber().Int32Value();
		auto width	   = info[2].ToNumber().Int32Value();
		auto height	   = info[3].ToNumber().Int32Value();
		auto format	   = info[4].ToNumber().Int32Value();
		auto framerate = info[5].ToNumber().Int32Value();
		if (!this->config_) return info.Env().Undefined();

		CallNativeFunc(
		  rs2_config_enable_stream,
		  &this->error_,
		  this->config_,
		  (rs2_stream) stream,
		  index,
		  width,
		  height,
		  (rs2_format) format,
		  framerate,
		  &this->error_);

		return info.This();
	}
};

Napi::FunctionReference RSConfig::constructor;

#endif
