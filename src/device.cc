#ifndef DEVICE_H
#define DEVICE_H

#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

FunctionReference RSDevice::constructor;

class RSDevice : public ObjectWrap<RSDevice> {
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
			InstanceMethod("querySensors", &RSDevice::QuerySensors),
			InstanceMethod("triggerErrorForTest", &RSDevice::TriggerErrorForTest),
			InstanceMethod("spawnRecorderDevice", &RSDevice::SpawnRecorderDevice),

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
			InstanceMethod("setStatusChangedCallbackMethodName", &RSDevice::SetStatusChangedCallbackMethodName),
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

	static Object NewInstance(rs2_device* dev, DeviceType type = kNormalDevice) {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		Object instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();

		auto me   = Nan::ObjectWrap::Unwrap<RSDevice>(instance);
		me->dev_  = dev;
		me->type_ = type;

		return scope.Escape(instance);
	}

  private:
	explicit RSDevice(DeviceType type = kNormalDevice)
	  : dev_(nullptr)
	  , error_(nullptr)
	  , type_(type) {
	}

	~RSDevice() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (dev_) rs2_delete_device(dev_);
		dev_ = nullptr;
	}

	static void New(const Napi::CallbackInfo& info) {
		if (info.IsConstructCall()) {
			RSDevice* obj = new RSDevice();
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	static Value GetCameraInfo(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		int32_t camera_info = info[0]->IntegerValue();
		;
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		auto value = GetNativeResult<
		  const char*>(rs2_get_device_info, &me->error_, me->dev_, static_cast<rs2_camera_info>(camera_info), &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(value).ToLocalChecked());
	}

	static Value Destroy(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (me) { me->DestroyMe(); }
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static Value SupportsCameraInfo(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::False());
		int32_t camera_info = info[0]->IntegerValue();
		auto me				= Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		int32_t on = GetNativeResult<
		  int>(rs2_supports_device_info, &me->error_, me->dev_, (rs2_camera_info) camera_info, &me->error_);
		if (me->error_) return;
		info.GetReturnValue().Set(Nan::New(on ? true : false));
	}

	static Value Reset(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_hardware_reset, &me->error_, me->dev_, &me->error_);
	}

	static Value QuerySensors(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		std::shared_ptr<rs2_sensor_list>
		list(GetNativeResult<rs2_sensor_list*>(rs2_query_sensors, &me->error_, me->dev_, &me->error_), rs2_delete_sensor_list);
		if (!list) return;

		auto size = GetNativeResult<int>(rs2_get_sensors_count, &me->error_, list.get(), &me->error_);
		if (!size) return;

		v8::Local<v8::Array> array = Nan::New<v8::Array>();
		for (int32_t i = 0; i < size; i++) {
			rs2_sensor* sensor
			  = GetNativeResult<rs2_sensor*>(rs2_create_sensor, &me->error_, list.get(), i, &me->error_);
			array->Set(i, RSSensor::NewInstance(sensor));
		}
		info.GetReturnValue().Set(array);
	}

	static Value TriggerErrorForTest(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		uint8_t raw_data[24] = { 0 };
		raw_data[0]			 = 0x14;
		raw_data[2]			 = 0xab;
		raw_data[3]			 = 0xcd;
		raw_data[4]			 = 0x4d;
		raw_data[8]			 = 4;
		CallNativeFunc(rs2_send_and_receive_raw_data, &me->error_, me->dev_, static_cast<void*>(raw_data), 24, &me->error_);
	}

	static Value SpawnRecorderDevice(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		v8::String::Utf8Value file(info[0]->ToString());
		auto dev = GetNativeResult<rs2_device*>(rs2_create_record_device, &me->error_, me->dev_, *file, &me->error_);
		if (me->error_) return;

		auto obj = RSDevice::NewInstance(dev, kRecorderDevice);
		info.GetReturnValue().Set(obj);
	}

	static Value PauseRecord(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_record_device_pause, &me->error_, me->dev_, &me->error_);
	}

	static Value ResumeRecord(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_record_device_resume, &me->error_, me->dev_, &me->error_);
	}

	static Value GetFileName(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		const char* file = nullptr;
		if (me->IsPlaybackInternal()) {
			file = GetNativeResult<const char*>(rs2_playback_device_get_file_path, &me->error_, me->dev_, &me->error_);
		}
		else if (me->IsRecorderInternal()) {
			file = GetNativeResult<const char*>(rs2_record_device_filename, &me->error_, me->dev_, &me->error_);
		}
		else {
			return;
		}
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(file).ToLocalChecked());
	}

	static Value PausePlayback(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_playback_device_pause, &me->error_, me->dev_, &me->error_);
	}

	static Value ResumePlayback(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_playback_device_resume, &me->error_, me->dev_, &me->error_);
	}

	static Value StopPlayback(const Napi::CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_playback_device_stop, &me->error_, me->dev_, &me->error_);
	}

	static Value GetPosition(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto pos = static_cast<uint32_t>(
		  GetNativeResult<uint32_t>(rs2_playback_get_position, &me->error_, me->dev_, &me->error_) / 1000000);
		info.GetReturnValue().Set(Nan::New(pos));
	}

	static Value GetDuration(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto duration = static_cast<uint32_t>(
		  GetNativeResult<uint32_t>(rs2_playback_get_duration, &me->error_, me->dev_, &me->error_) / 1000000);
		info.GetReturnValue().Set(Nan::New(duration));
	}

	static Value Seek(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		uint64_t time = info[0]->IntegerValue();
		CallNativeFunc(rs2_playback_seek, &me->error_, me->dev_, time * 1000000, &me->error_);
	}

	static Value IsRealTime(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto val = GetNativeResult<int>(rs2_playback_device_is_real_time, &me->error_, me->dev_, &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(val ? Nan::True() : Nan::False());
	}

	static Value SetIsRealTime(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto val = info[0]->BooleanValue();
		CallNativeFunc(rs2_playback_device_set_real_time, &me->error_, me->dev_, val, &me->error_);
	}

	static Value SetPlaybackSpeed(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto speed = info[0]->NumberValue();
		CallNativeFunc(rs2_playback_device_set_playback_speed, &me->error_, me->dev_, speed, &me->error_);
	}

	static Value IsPlayback(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto val = me->IsPlaybackInternal();
		info.GetReturnValue().Set(val ? Nan::True() : Nan::False());
	}

	static Value IsRecorder(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto val = me->IsRecorderInternal();
		info.GetReturnValue().Set(val ? Nan::True() : Nan::False());
	}

	static Value GetCurrentStatus(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto status = GetNativeResult<
		  rs2_playback_status>(rs2_playback_device_get_current_status, &me->error_, me->dev_, &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(status));
	}

	static Value SetStatusChangedCallbackMethodName(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		v8::String::Utf8Value method(info[0]->ToString());
		me->status_changed_callback_method_name_ = std::string(*method);
		CallNativeFunc(
		  rs2_playback_device_set_status_changed_callback,
		  &me->error_,
		  me->dev_,
		  new PlaybackStatusCallback(me),
		  &me->error_);
	}

	static Value IsTm2(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto val
		  = GetNativeResult<int>(rs2_is_device_extendable_to, &me->error_, me->dev_, RS2_EXTENSION_TM2, &me->error_);
		info.GetReturnValue().Set(val ? Nan::True() : Nan::False());
	}

	static Value EnableLoopback(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		v8::String::Utf8Value file(info[0]->ToString());
		CallNativeFunc(rs2_loopback_enable, &me->error_, me->dev_, *file, &me->error_);
	}

	static Value DisableLoopback(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		CallNativeFunc(rs2_loopback_disable, &me->error_, me->dev_, &me->error_);
	}

	static Value IsLoopbackEnabled(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto val = GetNativeResult<int>(rs2_loopback_is_enabled, &me->error_, me->dev_, &me->error_);
		info.GetReturnValue().Set(val ? Nan::True() : Nan::False());
	}

	static Value ConnectController(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto array_buffer = v8::Local<v8::ArrayBuffer>::Cast(info[0]);
		auto contents	 = array_buffer->GetContents();
		CallNativeFunc(rs2_connect_tm2_controller, &me->error_, me->dev_, static_cast<const uint8_t*>(contents.Data()), &me->error_);
	}

	static Value DisconnectController(const Napi::CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSDevice>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me) return;

		auto id = info[0]->IntegerValue();
		CallNativeFunc(rs2_disconnect_tm2_controller, &me->error_, me->dev_, id, &me->error_);
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


#endif
