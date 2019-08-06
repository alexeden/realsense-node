#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>
#include "main_thread_callback.cc"
#include "options.cc"
#include "utils.cc"

using namespace Napi;


class RSSensor
  : public ObjectWrap<RSSensor>
  , Options {
  public:
	static void Init(Object exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSSensor").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "openStream", OpenStream);
		Nan::SetPrototypeMethod(tpl, "openMultipleStream", OpenMultipleStream);
		Nan::SetPrototypeMethod(tpl, "getCameraInfo", GetCameraInfo);
		Nan::SetPrototypeMethod(tpl, "startWithSyncer", StartWithSyncer);
		Nan::SetPrototypeMethod(tpl, "startWithCallback", StartWithCallback);
		Nan::SetPrototypeMethod(tpl, "supportsOption", SupportsOption);
		Nan::SetPrototypeMethod(tpl, "getOption", GetOption);
		Nan::SetPrototypeMethod(tpl, "setOption", SetOption);
		Nan::SetPrototypeMethod(tpl, "getOptionRange", GetOptionRange);
		Nan::SetPrototypeMethod(tpl, "isOptionReadonly", IsOptionReadonly);
		Nan::SetPrototypeMethod(tpl, "getOptionDescription", GetOptionDescription);
		Nan::SetPrototypeMethod(tpl, "getOptionValueDescription", GetOptionValueDescription);
		Nan::SetPrototypeMethod(tpl, "stop", Stop);
		Nan::SetPrototypeMethod(tpl, "supportsCameraInfo", SupportsCameraInfo);
		Nan::SetPrototypeMethod(tpl, "getStreamProfiles", GetStreamProfiles);
		Nan::SetPrototypeMethod(tpl, "close", Close);
		Nan::SetPrototypeMethod(tpl, "setNotificationCallback", SetNotificationCallback);
		Nan::SetPrototypeMethod(tpl, "setRegionOfInterest", SetRegionOfInterest);
		Nan::SetPrototypeMethod(tpl, "getRegionOfInterest", GetRegionOfInterest);
		Nan::SetPrototypeMethod(tpl, "getDepthScale", GetDepthScale);
		Nan::SetPrototypeMethod(tpl, "isDepthSensor", IsDepthSensor);
		Nan::SetPrototypeMethod(tpl, "isROISensor", IsROISensor);
		constructor.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSSensor").ToLocalChecked(), tpl->GetFunction());
	}

	static Object NewInstance(rs2_sensor* sensor) {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		Object instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();

		auto me		= Nan::ObjectWrap::Unwrap<RSSensor>(instance);
		this->sensor_ = sensor;
		return scope.Escape(instance);
	}

	rs2_options* GetOptionsPointer() override {
		// TODO(shaoting) find better way to avoid the reinterpret_cast which was
		// caused the inheritance relation was hidden
		return reinterpret_cast<rs2_options*>(sensor_);
	}

	void ReplaceFrame(rs2_frame* raw_frame) {
		// clear old frame first.
		frame_->Replace(nullptr);
		video_frame_->Replace(nullptr);
		depth_frame_->Replace(nullptr);
		disparity_frame_->Replace(nullptr);
		motion_frame_->Replace(nullptr);
		pose_frame_->Replace(nullptr);

		if (GetNativeResult<
			  int>(rs2_is_frame_extendable_to, &error_, raw_frame, RS2_EXTENSION_DISPARITY_FRAME, &error_)) {
			disparity_frame_->Replace(raw_frame);
		}
		else if (GetNativeResult<
				   int>(rs2_is_frame_extendable_to, &error_, raw_frame, RS2_EXTENSION_DEPTH_FRAME, &error_)) {
			depth_frame_->Replace(raw_frame);
		}
		else if (GetNativeResult<
				   int>(rs2_is_frame_extendable_to, &error_, raw_frame, RS2_EXTENSION_VIDEO_FRAME, &error_)) {
			video_frame_->Replace(raw_frame);
		}
		else if (GetNativeResult<
				   int>(rs2_is_frame_extendable_to, &error_, raw_frame, RS2_EXTENSION_MOTION_FRAME, &error_)) {
			motion_frame_->Replace(raw_frame);
		}
		else if (GetNativeResult<
				   int>(rs2_is_frame_extendable_to, &error_, raw_frame, RS2_EXTENSION_POSE_FRAME, &error_)) {
			pose_frame_->Replace(raw_frame);
		}
		else {
			frame_->Replace(raw_frame);
		}
	}

  private:
	RSSensor()
	  : sensor_(nullptr)
	  , error_(nullptr)
	  , profile_list_(nullptr)
	  , frame_(nullptr)
	  , video_frame_(nullptr)
	  , depth_frame_(nullptr)
	  , disparity_frame_(nullptr)
	  , motion_frame_(nullptr)
	  , pose_frame_(nullptr) {
	}

	~RSSensor() {
		DestroyMe();
	}

	void RegisterNotificationCallbackMethod();

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (sensor_) rs2_delete_sensor(sensor_);
		sensor_ = nullptr;
		if (profile_list_) rs2_delete_stream_profiles_list(profile_list_);
		profile_list_ = nullptr;
	}

	static void New(const CallbackInfo& info) {
		if (info.IsConstructCall()) {
			RSSensor* obj = new RSSensor();
			obj->Wrap(info.This());
			return info.This();
		}
	}

	Napi::Value SupportsOption(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return this->SupportsOptionInternal(info);

		return Boolean::New(info.Env(), false);
	}

	Napi::Value GetOption(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return this->GetOptionInternal(info);

		return info.Env().Undefined();
	}

	Napi::Value GetOptionDescription(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return this->GetOptionDescriptionInternal(info);

		return info.Env().Undefined();
	}

	Napi::Value GetOptionValueDescription(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return this->GetOptionValueDescriptionInternal(info);

		return info.Env().Undefined();
	}

	Napi::Value SetOption(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return this->SetOptionInternal(info);

		return info.Env().Undefined();
	}

	Napi::Value GetOptionRange(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return this->GetOptionRangeInternal(info);

		return info.Env().Undefined();
	}

	Napi::Value IsOptionReadonly(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return this->IsOptionReadonlyInternal(info);

		return Boolean::New(info.Env(), false);
	}

	Napi::Value GetCameraInfo(const CallbackInfo& info) {
		return info.Env().Undefined();
		int32_t camera_info = info[0]->IntegerValue();
		;
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		auto value = GetNativeResult<
		  const char*>(rs2_get_sensor_info, &this->error_, this->sensor_, static_cast<rs2_camera_info>(camera_info), &this->error_);
		if (this->error_) return;

		info.GetReturnValue().Set(Nan::New(value).ToLocalChecked());
	}

	Napi::Value StartWithSyncer(const CallbackInfo& info) {
		return info.Env().Undefined();
		auto syncer = Nan::ObjectWrap::Unwrap<RSSyncer>(info[0]->ToObject());
		auto me		= Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me || !syncer) return;

		CallNativeFunc(rs2_start_cpp, &this->error_, this->sensor_, new FrameCallbackForProcessingBlock(syncer->syncer_), &this->error_);
	}

	Napi::Value StartWithCallback(const CallbackInfo& info) {
		auto frame			 = Nan::ObjectWrap::Unwrap<RSFrame>(info[1]->ToObject());
		auto depth_frame	 = Nan::ObjectWrap::Unwrap<RSFrame>(info[2]->ToObject());
		auto video_frame	 = Nan::ObjectWrap::Unwrap<RSFrame>(info[3]->ToObject());
		auto disparity_frame = Nan::ObjectWrap::Unwrap<RSFrame>(info[4]->ToObject());
		auto motion_frame	= Nan::ObjectWrap::Unwrap<RSFrame>(info[5]->ToObject());
		auto pose_frame		 = Nan::ObjectWrap::Unwrap<RSFrame>(info[6]->ToObject());
		auto me				 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me && frame && depth_frame && video_frame && disparity_frame && motion_frame && pose_frame) {
			this->frame_			 = frame;
			this->depth_frame_	 = depth_frame;
			this->video_frame_	 = video_frame;
			this->disparity_frame_ = disparity_frame;
			this->motion_frame_	= motion_frame;
			this->pose_frame_		 = pose_frame;
			v8::String::Utf8Value str(info[0]);
			this->frame_callback_name_ = std::string(*str);
			CallNativeFunc(rs2_start_cpp, &this->error_, this->sensor_, new FrameCallbackForProc(me), &this->error_);
		}
		return info.Env().Undefined();
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) { this->DestroyMe(); }
		return info.Env().Undefined();
	}

	Napi::Value OpenStream(const CallbackInfo& info) {
		return info.Env().Undefined();
		auto me		 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		auto profile = Nan::ObjectWrap::Unwrap<RSStreamProfile>(info[0]->ToObject());
		if (!me || !profile) return;

		CallNativeFunc(rs2_open, &this->error_, this->sensor_, profile->profile_, &this->error_);
	}

	Napi::Value OpenMultipleStream(const CallbackInfo& info) {
		return info.Env().Undefined();
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		auto array   = v8::Local<v8::Array>::Cast(info[0]);
		uint32_t len = array->Length();
		std::vector<const rs2_stream_profile*> profs;
		for (uint32_t i = 0; i < len; i++) {
			auto profile = Nan::ObjectWrap::Unwrap<RSStreamProfile>(array->Get(i)->ToObject());
			profs.push_back(profile->profile_);
		}
		CallNativeFunc(rs2_open_multiple, &this->error_, this->sensor_, profs.data(), len, &this->error_);
	}

	Napi::Value Stop(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_stop, &this->error_, this->sensor_, &this->error_);
	}

	Napi::Value GetStreamProfiles(const CallbackInfo& info) {
		return info.Env().Undefined();
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		rs2_stream_profile_list* list = this->profile_list_;
		if (!list) {
			list = GetNativeResult<
			  rs2_stream_profile_list*>(rs2_get_stream_profiles, &this->error_, this->sensor_, &this->error_);
			this->profile_list_ = list;
		}
		if (!list) return;

		int32_t size = GetNativeResult<int>(rs2_get_stream_profiles_count, &this->error_, list, &this->error_);
		v8::Local<v8::Array> array = Nan::New<v8::Array>(size);
		for (int32_t i = 0; i < size; i++) {
			rs2_stream_profile* profile = const_cast<rs2_stream_profile*>(rs2_get_stream_profile(list, i, &this->error_));
			array->Set(i, RSStreamProfile::NewInstance(profile));
		}
		info.GetReturnValue().Set(array);
	}

	Napi::Value SupportsCameraInfo(const CallbackInfo& info) {
		info.GetReturnValue().Set(Nan::False());
		int32_t camera_info = info[0]->IntegerValue();
		auto me				= Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		int32_t on = GetNativeResult<
		  int>(rs2_supports_sensor_info, &this->error_, this->sensor_, (rs2_camera_info) camera_info, &this->error_);
		info.GetReturnValue().Set(Nan::New(on ? true : false));
	}

	Napi::Value Close(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_close, &this->error_, this->sensor_, &this->error_);
	}

	Napi::Value SetNotificationCallback(const CallbackInfo& info) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		v8::String::Utf8Value value(info[0]->ToString());
		this->notification_callback_name_ = std::string(*value);
		this->RegisterNotificationCallbackMethod();
	}

	Napi::Value SetRegionOfInterest(const CallbackInfo& info) {
		return info.Env().Undefined();
		int32_t minx = info[0]->IntegerValue();
		int32_t miny = info[1]->IntegerValue();
		int32_t maxx = info[2]->IntegerValue();
		int32_t maxy = info[3]->IntegerValue();
		auto me		 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_set_region_of_interest, &this->error_, this->sensor_, minx, miny, maxx, maxy, &this->error_);
	}

	Napi::Value GetRegionOfInterest(const CallbackInfo& info) {
		return info.Env().Undefined();
		int32_t minx = 0;
		int32_t miny = 0;
		int32_t maxx = 0;
		int32_t maxy = 0;
		auto me		 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_get_region_of_interest, &this->error_, this->sensor_, &minx, &miny, &maxx, &maxy, &this->error_);
		if (this->error_) return;
		info.GetReturnValue().Set(RSRegionOfInterest(minx, miny, maxx, maxy).GetObject());
	}

	Napi::Value GetDepthScale(const CallbackInfo& info) {
		return info.Env().Undefined();
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		auto scale = GetNativeResult<float>(rs2_get_depth_scale, &this->error_, this->sensor_, &this->error_);
		if (this->error_) return;

		info.GetReturnValue().Set(Nan::New(scale));
	}

	Napi::Value IsDepthSensor(const CallbackInfo& info) {
		return info.Env().Undefined();
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		bool is_depth = GetNativeResult<
		  int>(rs2_is_sensor_extendable_to, &this->error_, this->sensor_, RS2_EXTENSION_DEPTH_SENSOR, &this->error_);
		if (this->error_) return;

		info.GetReturnValue().Set(Nan::New(is_depth));
	}

	Napi::Value IsROISensor(const CallbackInfo& info) {
		return info.Env().Undefined();
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		bool is_roi
		  = GetNativeResult<int>(rs2_is_sensor_extendable_to, &this->error_, this->sensor_, RS2_EXTENSION_ROI, &this->error_);
		if (this->error_) return;

		info.GetReturnValue().Set(Nan::New(is_roi));
	}

  private:
	static FunctionReference constructor;
	rs2_sensor* sensor_;
	rs2_error* error_;
	rs2_stream_profile_list* profile_list_;
	std::string frame_callback_name_;
	std::string notification_callback_name_;
	RSFrame* frame_;
	RSFrame* video_frame_;
	RSFrame* depth_frame_;
	RSFrame* disparity_frame_;
	RSFrame* motion_frame_;
	RSFrame* pose_frame_;
	friend class RSContext;
	friend class DevicesChangedCallbackInfo;
	friend class FrameCallbackInfo;
	friend class NotificationCallbackInfo;
};

Napi::FunctionReference RSSensor::constructor;


void RSSensor::RegisterNotificationCallbackMethod() {
	CallNativeFunc(rs2_set_notifications_callback_cpp, &error_, sensor_, new NotificationCallback(this), &error_);
}

#endif
