#ifndef SENSOR_H
#define SENSOR_H

#include "dicts.cc"
#include "frame.cc"
#include "notification_callbacks.cc"
#include "options.cc"
#include "syncer.cc"
#include "utils.cc"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSSensor
  : public ObjectWrap<RSSensor>
  , Options {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSSensor",
		  {
			InstanceMethod("close", &RSSensor::Close),
			InstanceMethod("destroy", &RSSensor::Destroy),
			InstanceMethod("getCameraInfo", &RSSensor::GetCameraInfo),
			InstanceMethod("getDepthScale", &RSSensor::GetDepthScale),
			InstanceMethod("getOption", &RSSensor::GetOption),
			InstanceMethod("getOptionDescription", &RSSensor::GetOptionDescription),
			InstanceMethod("getOptionRange", &RSSensor::GetOptionRange),
			InstanceMethod("getOptionValueDescription", &RSSensor::GetOptionValueDescription),
			InstanceMethod("getRegionOfInterest", &RSSensor::GetRegionOfInterest),
			InstanceMethod("getStreamProfiles", &RSSensor::GetStreamProfiles),
			InstanceMethod("isDepthSensor", &RSSensor::IsDepthSensor),
			InstanceMethod("isOptionReadonly", &RSSensor::IsOptionReadonly),
			InstanceMethod("isROISensor", &RSSensor::IsROISensor),
			InstanceMethod("onNotification", &RSSensor::OnNotification),
			InstanceMethod("openMultipleStream", &RSSensor::OpenMultipleStream),
			InstanceMethod("openStream", &RSSensor::OpenStream),
			InstanceMethod("setOption", &RSSensor::SetOption),
			InstanceMethod("setRegionOfInterest", &RSSensor::SetRegionOfInterest),
			InstanceMethod("startWithCallback", &RSSensor::StartWithCallback),
			InstanceMethod("startWithSyncer", &RSSensor::StartWithSyncer),
			InstanceMethod("stop", &RSSensor::Stop),
			InstanceMethod("supportsCameraInfo", &RSSensor::SupportsCameraInfo),
			InstanceMethod("supportsOption", &RSSensor::SupportsOption),
		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSSensor", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_sensor* sensor) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		auto unwrapped	   = ObjectWrap<RSSensor>::Unwrap(instance);
		unwrapped->sensor_ = sensor;

		return scope.Escape(napi_value(instance)).ToObject();
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
	RSSensor(const CallbackInfo& info)
	  : ObjectWrap<RSSensor>(info)
	  , sensor_(nullptr)
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

  private:
  	static FunctionReference constructor;
	rs2_sensor* sensor_;
	rs2_error* error_;
	rs2_stream_profile_list* profile_list_;
	std::string frame_callback_name_;
	RSFrame* frame_;
	RSFrame* video_frame_;
	RSFrame* depth_frame_;
	RSFrame* disparity_frame_;
	RSFrame* motion_frame_;
	RSFrame* pose_frame_;
	friend class RSContext;
	friend class FrameCallbackInfo;

    void RegisterNotificationCallbackMethod(std::shared_ptr<ThreadSafeCallback> callback) {
        rs2_set_notifications_callback_cpp(sensor_, new NotificationCallback(callback), &this->error_);
    }

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (sensor_) rs2_delete_sensor(sensor_);
		sensor_ = nullptr;
		if (profile_list_) rs2_delete_stream_profiles_list(profile_list_);
		profile_list_ = nullptr;
	}

	Napi::Value Close(const CallbackInfo& info) {
		CallNativeFunc(rs2_close, &this->error_, this->sensor_, &this->error_);
		return info.This();
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.This();
	}

	Napi::Value GetCameraInfo(const CallbackInfo& info) {
		int32_t camera_info = info[0].ToNumber().Int32Value();

		auto value = GetNativeResult<const char*>(
		  rs2_get_sensor_info, &this->error_, this->sensor_, static_cast<rs2_camera_info>(camera_info), &this->error_);
		if (this->error_) return info.Env().Undefined();

		return String::New(info.Env(), value);
	}

	Napi::Value GetDepthScale(const CallbackInfo& info) {
		auto scale = GetNativeResult<float>(rs2_get_depth_scale, &this->error_, this->sensor_, &this->error_);
		if (this->error_) return info.Env().Undefined();

		return Number::New(info.Env(), scale);
	}

	Napi::Value GetOption(const CallbackInfo& info) {
		return this->GetOptionInternal(info);
	}

	Napi::Value GetOptionDescription(const CallbackInfo& info) {
		return this->GetOptionDescriptionInternal(info);
	}

	Napi::Value GetOptionRange(const CallbackInfo& info) {
		return this->GetOptionRangeInternal(info);
	}

	Napi::Value GetOptionValueDescription(const CallbackInfo& info) {
		return this->GetOptionValueDescriptionInternal(info);
	}

	Napi::Value GetRegionOfInterest(const CallbackInfo& info) {
		int32_t minx = 0;
		int32_t miny = 0;
		int32_t maxx = 0;
		int32_t maxy = 0;

		CallNativeFunc(rs2_get_region_of_interest, &this->error_, this->sensor_, &minx, &miny, &maxx, &maxy, &this->error_);
		if (this->error_) return info.Env().Undefined();
		return RSRegionOfInterest(info.Env(), minx, miny, maxx, maxy).GetObject();
	}

	Napi::Value GetStreamProfiles(const CallbackInfo& info) {
		rs2_stream_profile_list* list = this->profile_list_;
		if (!list) {
			list = GetNativeResult<
			  rs2_stream_profile_list*>(rs2_get_stream_profiles, &this->error_, this->sensor_, &this->error_);
			this->profile_list_ = list;
		}
		if (!list) return info.Env().Undefined();

		int32_t size = GetNativeResult<int>(rs2_get_stream_profiles_count, &this->error_, list, &this->error_);
		auto array	 = Array::New(info.Env());
		for (int32_t i = 0; i < size; i++) {
			rs2_stream_profile* profile
			  = const_cast<rs2_stream_profile*>(rs2_get_stream_profile(list, i, &this->error_));
			array.Set(i, RSStreamProfile::NewInstance(info.Env(), profile));
		}

		return array;
	}

	Napi::Value IsDepthSensor(const CallbackInfo& info) {
		bool is_depth = GetNativeResult<
		  int>(rs2_is_sensor_extendable_to, &this->error_, this->sensor_, RS2_EXTENSION_DEPTH_SENSOR, &this->error_);
		if (this->error_) return info.Env().Undefined();

		return Boolean::New(info.Env(), is_depth);
	}

	Napi::Value IsOptionReadonly(const CallbackInfo& info) {
		return this->IsOptionReadonlyInternal(info);
	}

	Napi::Value IsROISensor(const CallbackInfo& info) {
		bool is_roi = GetNativeResult<
		  int>(rs2_is_sensor_extendable_to, &this->error_, this->sensor_, RS2_EXTENSION_ROI, &this->error_);
		if (this->error_) return info.Env().Undefined();

		return Boolean::New(info.Env(), is_roi);
	}

    Napi::Value OnNotification(const CallbackInfo& info) {
        auto callback = std::make_shared<ThreadSafeCallback>(info[0].As<Function>());

		this->RegisterNotificationCallbackMethod(callback);
		return info.This();
	}

	Napi::Value OpenMultipleStream(const CallbackInfo& info) {
		auto array	 = info[0].As<Array>();
		uint32_t len = array.Length();
		std::vector<const rs2_stream_profile*> profs;
		for (uint32_t i = 0; i < len; i++) {
			auto profile = ObjectWrap<RSStreamProfile>::Unwrap(array.Get(i).ToObject());
			profs.push_back(profile->profile_);
		}
		CallNativeFunc(rs2_open_multiple, &this->error_, this->sensor_, profs.data(), len, &this->error_);
		return info.This();
	}

	Napi::Value OpenStream(const CallbackInfo& info) {
		auto profile = ObjectWrap<RSStreamProfile>::Unwrap(info[0].ToObject());
		if (!profile) return info.Env().Undefined();

		CallNativeFunc(rs2_open, &this->error_, this->sensor_, profile->profile_, &this->error_);
		return info.This();
	}

	Napi::Value SetOption(const CallbackInfo& info) {
		this->SetOptionInternal(info);

		return info.This();
	}

	Napi::Value SetRegionOfInterest(const CallbackInfo& info) {
		int32_t minx = info[0].ToNumber().Int32Value();
		int32_t miny = info[1].ToNumber().Int32Value();
		int32_t maxx = info[2].ToNumber().Int32Value();
		int32_t maxy = info[3].ToNumber().Int32Value();

		CallNativeFunc(rs2_set_region_of_interest, &this->error_, this->sensor_, minx, miny, maxx, maxy, &this->error_);
		return info.This();
	}

	Napi::Value StartWithCallback(const CallbackInfo& info) {
        auto callback = std::make_shared<ThreadSafeCallback>(info[0].As<Function>());
		auto frame			 = ObjectWrap<RSFrame>::Unwrap(info[1].ToObject());
		auto depth_frame	 = ObjectWrap<RSFrame>::Unwrap(info[2].ToObject());
		auto video_frame	 = ObjectWrap<RSFrame>::Unwrap(info[3].ToObject());
		auto disparity_frame = ObjectWrap<RSFrame>::Unwrap(info[4].ToObject());
		auto motion_frame	 = ObjectWrap<RSFrame>::Unwrap(info[5].ToObject());
		auto pose_frame		 = ObjectWrap<RSFrame>::Unwrap(info[6].ToObject());
		if (frame && depth_frame && video_frame && disparity_frame && motion_frame && pose_frame) {
			this->frame_			   = frame;
			this->depth_frame_		   = depth_frame;
			this->video_frame_		   = video_frame;
			this->disparity_frame_	   = disparity_frame;
			this->motion_frame_		   = motion_frame;
			this->pose_frame_		   = pose_frame;
			// this->frame_callback_name_ = std::string(info[0].ToString());
			CallNativeFunc(rs2_start_cpp, &this->error_, this->sensor_, new FrameCallbackForProc(callback, this), &this->error_);
		}
		return info.Env().Undefined();
	}

	Napi::Value StartWithSyncer(const CallbackInfo& info) {
		auto syncer = ObjectWrap<RSSyncer>::Unwrap(info[0].ToObject());
		if (!syncer) return info.Env().Undefined();

		CallNativeFunc(
		  rs2_start_cpp,
		  &this->error_,
		  this->sensor_,
		  new FrameCallbackForProcessingBlock(syncer->syncer_),
		  &this->error_);

		return info.This();
	}

	Napi::Value Stop(const CallbackInfo& info) {
		CallNativeFunc(rs2_stop, &this->error_, this->sensor_, &this->error_);
		return info.This();
	}

	Napi::Value SupportsCameraInfo(const CallbackInfo& info) {
		int32_t camera_info = info[0].ToNumber().Int32Value();
		int32_t on			= GetNativeResult<
		   int>(rs2_supports_sensor_info, &this->error_, this->sensor_, (rs2_camera_info) camera_info, &this->error_);

		return Boolean::New(info.Env(), on ? true : false);
	}

	Napi::Value SupportsOption(const CallbackInfo& info) {
		return this->SupportsOptionInternal(info);
	}
};

Napi::FunctionReference RSSensor::constructor;

#endif
