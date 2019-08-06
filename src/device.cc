#ifndef DEVICE_H
#define DEVICE_H

#include "main_thread_callback.cc"
// #include "sensor.cc"
#include "utils.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;


class RSDevice : public Napi::ObjectWrap<RSDevice> {
  public:
	enum DeviceType {
		kNormalDevice = 0,
		kRecorderDevice,
		kPlaybackDevice,
	};

	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSDevice",
		  {
			InstanceMethod("destroy", &RSDevice::Destroy),
			InstanceMethod("getCameraInfo", &RSDevice::GetCameraInfo),
			InstanceMethod("supportsCameraInfo", &RSDevice::SupportsCameraInfo),
			InstanceMethod("reset", &RSDevice::Reset),
			// InstanceMethod("querySensors", &RSDevice::QuerySensors),
			InstanceMethod("triggerErrorForTest", &RSDevice::TriggerErrorForTest),
			// InstanceMethod("spawnRecorderDevice", &RSDevice::SpawnRecorderDevice),

			// Methods for record or playback
			InstanceMethod("pauseRecord", &RSDevice::PauseRecord),
			InstanceMethod("resumeRecord", &RSDevice::ResumeRecord),
			InstanceMethod("getFileName", &RSDevice::GetFileName),
			InstanceMethod("pausePlayback", &RSDevice::PausePlayback),
			InstanceMethod("resumePlayback", &RSDevice::ResumePlayback),
			InstanceMethod("stopPlayback", &RSDevice::StopPlayback),
			InstanceMethod("getPosition", &RSDevice::GetPosition),
			InstanceMethod("getDuration", &RSDevice::GetDuration),
			InstanceMethod("seek", &RSDevice::Seek),
			InstanceMethod("isRealTime", &RSDevice::IsRealTime),
			InstanceMethod("setIsRealTime", &RSDevice::SetIsRealTime),
			InstanceMethod("setPlaybackSpeed", &RSDevice::SetPlaybackSpeed),
			InstanceMethod("getCurrentStatus", &RSDevice::GetCurrentStatus),
			// InstanceMethod("setStatusChangedCallbackMethodName", &RSDevice::SetStatusChangedCallbackMethodName),
			InstanceMethod("isTm2", &RSDevice::IsTm2),
			InstanceMethod("isPlayback", &RSDevice::IsPlayback),
			InstanceMethod("isRecorder", &RSDevice::IsRecorder),

			// methods of tm2 device
			InstanceMethod("enableLoopback", &RSDevice::EnableLoopback),
			InstanceMethod("disableLoopback", &RSDevice::DisableLoopback),
			InstanceMethod("isLoopbackEnabled", &RSDevice::IsLoopbackEnabled),
			InstanceMethod("connectController", &RSDevice::ConnectController),
			InstanceMethod("disconnectController", &RSDevice::DisconnectController),

		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSDevice", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_device* dev, DeviceType type = kNormalDevice) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		return scope.Escape(napi_value(instance)).ToObject();

		// v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor);
		// v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		// Object instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();

		// me->dev_  = dev;
		// me->type_ = type;

		// return scope.Escape(instance);
	}

	// explicit RSDevice(DeviceType type = kNormalDevice)
	RSDevice(const CallbackInfo& info)
	  : Napi::ObjectWrap<RSDevice>(info)
	  , dev_(nullptr)
	  , error_(nullptr) {
		type_ = static_cast<DeviceType>(info[0].As<Number>().Uint32Value());
	}

	~RSDevice() {
		DestroyMe();
	}

  private:
	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (dev_) rs2_delete_device(dev_);
		dev_ = nullptr;
	}

	Napi::Value GetCameraInfo(const Napi::CallbackInfo& info) {
		int32_t camera_info = info[0].As<Number>().Int32Value();

		auto value = GetNativeResult<
		  const char*>(rs2_get_device_info, &this->error_, this->dev_, static_cast<rs2_camera_info>(camera_info), &this->error_);
		if (this->error_) throw Napi::Error::New(info.Env(), "Error trying to get camera info");

		return String::New(info.Env(), value);
	}

	Napi::Value Destroy(const Napi::CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	Napi::Value SupportsCameraInfo(const Napi::CallbackInfo& info) {
		int32_t camera_info = info[0].As<Number>().Int32Value();
		int32_t on			= GetNativeResult<
		   int>(rs2_supports_device_info, &this->error_, this->dev_, (rs2_camera_info) camera_info, &this->error_);
		if (this->error_) throw Napi::Error::New(info.Env(), "Error trying to get camera support info");;

		return on ? Boolean::New(info.Env(), true) : Boolean::New(info.Env(), false);
	}

	Napi::Value Reset(const Napi::CallbackInfo& info) {
		CallNativeFunc(rs2_hardware_reset, &this->error_, this->dev_, &this->error_);
		return info.This();
	}

	// Napi::Value QuerySensors(const Napi::CallbackInfo& info) {
	// 	std::shared_ptr<rs2_sensor_list>
	// 	list(GetNativeResult<rs2_sensor_list*>(rs2_query_sensors, &this->error_, this->dev_, &this->error_), rs2_delete_sensor_list);
	// 	if (!list) return;

	// 	auto size = GetNativeResult<int>(rs2_get_sensors_count, &this->error_, list.get(), &this->error_);
	// 	if (!size) return;

	// 	auto array = Napi::Array::New(info.Env());
	// 	for (int32_t i = 0; i < size; i++) {
	// 		rs2_sensor* sensor
	// 		  = GetNativeResult<rs2_sensor*>(rs2_create_sensor, &this->error_, list.get(), i, &this->error_);
	// 		array.Set(i, RSSensor::NewInstance(sensor));
	// 	}

	// 	return array;
	// }

	Napi::Value TriggerErrorForTest(const Napi::CallbackInfo& info) {
		uint8_t raw_data[24] = { 0 };
		raw_data[0]			 = 0x14;
		raw_data[2]			 = 0xab;
		raw_data[3]			 = 0xcd;
		raw_data[4]			 = 0x4d;
		raw_data[8]			 = 4;
		CallNativeFunc(rs2_send_and_receive_raw_data, &this->error_, this->dev_, static_cast<void*>(raw_data), 24, &this->error_);

		return info.Env().Undefined();
	}

	// Napi::Value SpawnRecorderDevice(const Napi::CallbackInfo& info) {
	// 	std::string file = info[0].As<String>().ToString();
	// 	auto dev
	// 	  = GetNativeResult<rs2_device*>(rs2_create_record_device, &this->error_, this->dev_, file, &this->error_);
	// 	if (this->error_) return;

	// 	return RSDevice::NewInstance(dev, kRecorderDevice);
	// }

	Napi::Value PauseRecord(const Napi::CallbackInfo& info) {
		CallNativeFunc(rs2_record_device_pause, &this->error_, this->dev_, &this->error_);
		return info.This();
	}

	Napi::Value ResumeRecord(const Napi::CallbackInfo& info) {
		CallNativeFunc(rs2_record_device_resume, &this->error_, this->dev_, &this->error_);
		return info.This();
	}

	Napi::Value GetFileName(const Napi::CallbackInfo& info) {
		const char* file = nullptr;
		if (this->IsPlaybackInternal()) {
			file = GetNativeResult<
			  const char*>(rs2_playback_device_get_file_path, &this->error_, this->dev_, &this->error_);
		}
		else if (this->IsRecorderInternal()) {
			file = GetNativeResult<const char*>(rs2_record_device_filename, &this->error_, this->dev_, &this->error_);
		}
		else {
			return String::New(info.Env(), "");
		}
		if (this->error_) throw Napi::Error::New(info.Env(), "Error trying to get filename");

		return String::New(info.Env(), file);
	}

	Napi::Value PausePlayback(const Napi::CallbackInfo& info) {
		CallNativeFunc(rs2_playback_device_pause, &this->error_, this->dev_, &this->error_);
		return info.This();
	}

	Napi::Value ResumePlayback(const Napi::CallbackInfo& info) {
		CallNativeFunc(rs2_playback_device_resume, &this->error_, this->dev_, &this->error_);
		return info.This();
	}

	Napi::Value StopPlayback(const Napi::CallbackInfo& info) {
		CallNativeFunc(rs2_playback_device_stop, &this->error_, this->dev_, &this->error_);
		return info.This();
	}

	Napi::Value GetPosition(const Napi::CallbackInfo& info) {
		auto pos = static_cast<uint32_t>(
		  GetNativeResult<uint32_t>(rs2_playback_get_position, &this->error_, this->dev_, &this->error_) / 1000000);

		return Number::New(info.Env(), pos);
	}

	Napi::Value GetDuration(const Napi::CallbackInfo& info) {
		auto duration = static_cast<uint32_t>(
		  GetNativeResult<uint32_t>(rs2_playback_get_duration, &this->error_, this->dev_, &this->error_) / 1000000);

		return Number::New(info.Env(), duration);
	}

	Napi::Value Seek(const Napi::CallbackInfo& info) {
		uint64_t time = info[0].As<Number>().Int32Value();
		CallNativeFunc(rs2_playback_seek, &this->error_, this->dev_, time * 1000000, &this->error_);

		return info.This();
	}

	Napi::Value IsRealTime(const Napi::CallbackInfo& info) {
		auto val = GetNativeResult<int>(rs2_playback_device_is_real_time, &this->error_, this->dev_, &this->error_);
		if (this->error_) throw Napi::Error::New(info.Env(), "Error trying to get device real-time state");;

		return val ? Boolean::New(info.Env(), true) : Boolean::New(info.Env(), false);
	}

	Napi::Value SetIsRealTime(const Napi::CallbackInfo& info) {
		auto val = info[0].As<Boolean>().ToBoolean();
		CallNativeFunc(rs2_playback_device_set_real_time, &this->error_, this->dev_, val, &this->error_);

		return info.This();
	}

	Napi::Value SetPlaybackSpeed(const Napi::CallbackInfo& info) {
		auto speed = info[0].As<Number>().ToNumber().Uint32Value();
		CallNativeFunc(rs2_playback_device_set_playback_speed, &this->error_, this->dev_, speed, &this->error_);

		return info.This();
	}

	Napi::Value IsPlayback(const Napi::CallbackInfo& info) {
		auto val = this->IsPlaybackInternal();
		return val ? Boolean::New(info.Env(), true) : Boolean::New(info.Env(), false);
	}

	Napi::Value IsRecorder(const Napi::CallbackInfo& info) {
		auto val = this->IsRecorderInternal();
		return val ? Boolean::New(info.Env(), true) : Boolean::New(info.Env(), false);
	}

	Napi::Value GetCurrentStatus(const Napi::CallbackInfo& info) {
		auto status = GetNativeResult<
		  rs2_playback_status>(rs2_playback_device_get_current_status, &this->error_, this->dev_, &this->error_);

		if (this->error_) throw Napi::Error::New(info.Env(), "Error trying to get current device status");

		return Number::New(info.Env(), status);
	}

	// Napi::Value SetStatusChangedCallbackMethodName(const Napi::CallbackInfo& info) {
	// 	std::string method						   = info[0].As<String>().ToString();
	// 	this->status_changed_callback_method_name_ = method;
	// 	CallNativeFunc(
	// 	  rs2_playback_device_set_status_changed_callback,
	// 	  &this->error_,
	// 	  this->dev_,
	// 	  new PlaybackStatusCallback(this),
	// 	  &this->error_);

	// 	return info.This();
	// }

	Napi::Value IsTm2(const Napi::CallbackInfo& info) {
		auto val = GetNativeResult<
		  int>(rs2_is_device_extendable_to, &this->error_, this->dev_, RS2_EXTENSION_TM2, &this->error_);
		return val ? Boolean::New(info.Env(), true) : Boolean::New(info.Env(), false);
	}

	Napi::Value EnableLoopback(const Napi::CallbackInfo& info) {
		std::string file = info[0].As<String>().ToString();
		CallNativeFunc(rs2_loopback_enable, &this->error_, this->dev_, file.c_str(), &this->error_);

        return info.Env().Undefined();
	}

	Napi::Value DisableLoopback(const Napi::CallbackInfo& info) {
		CallNativeFunc(rs2_loopback_disable, &this->error_, this->dev_, &this->error_);

        return info.Env().Undefined();
	}

	Napi::Value IsLoopbackEnabled(const Napi::CallbackInfo& info) {
		auto val = GetNativeResult<int>(rs2_loopback_is_enabled, &this->error_, this->dev_, &this->error_);
		return val ? Boolean::New(info.Env(), true) : Boolean::New(info.Env(), false);
	}

	Napi::Value ConnectController(const Napi::CallbackInfo& info) {
		auto array_buffer = info[0].As<ArrayBuffer>();
		CallNativeFunc(
		  rs2_connect_tm2_controller,
		  &this->error_,
		  this->dev_,
		  static_cast<const uint8_t*>(array_buffer.Data()),
		  &this->error_);

        return info.Env().Undefined();
	}

	Napi::Value DisconnectController(const Napi::CallbackInfo& info) {
		auto id = info[0].As<Number>().Int32Value();
		CallNativeFunc(rs2_disconnect_tm2_controller, &this->error_, this->dev_, id, &this->error_);
        return info.Env().Undefined();
	}

  private:
	bool IsPlaybackInternal() {
		auto val = GetNativeResult<int>(rs2_is_device_extendable_to, &error_, dev_, RS2_EXTENSION_PLAYBACK, &error_);

		return (error_ || !val) ? false : true;
	}

	bool IsRecorderInternal() {
		auto val = GetNativeResult<int>(rs2_is_device_extendable_to, &error_, dev_, RS2_EXTENSION_RECORD, &error_);

		return (error_ || !val) ? false : true;
	}
	static FunctionReference constructor;
	rs2_device* dev_;
	rs2_error* error_;
	DeviceType type_;
	std::string status_changed_callback_method_name_;
	friend class RSContext;
	friend class DevicesChangedCallbackInfo;
	friend class FrameCallbackInfo;
	friend class RSPipeline;
	friend class RSDeviceList;
	friend class RSDeviceHub;
	friend class PlaybackStatusCallbackInfo;
};

FunctionReference RSDevice::constructor;

#endif
