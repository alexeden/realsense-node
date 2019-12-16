#ifndef STREAM_PROFILE_H
#define STREAM_PROFILE_H

#include "dicts.cc"
#include "utils.cc"
#include <iostream>
#include <librealsense2/h/rs_internal.h>
#include <librealsense2/hpp/rs_types.hpp>
#include <librealsense2/rs.h>
#include <napi.h>

using namespace Napi;

class RSStreamProfile : public ObjectWrap<RSStreamProfile> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSStreamProfile",
		  {
			InstanceAccessor("format", &RSStreamProfile::Format, nullptr),
			InstanceAccessor("fps", &RSStreamProfile::Fps, nullptr),
			InstanceAccessor("height", &RSStreamProfile::Height, nullptr),
			InstanceAccessor("index", &RSStreamProfile::Index, nullptr),
			InstanceAccessor("isDefault", &RSStreamProfile::IsDefault, nullptr),
			InstanceAccessor("isMotionProfile", &RSStreamProfile::IsMotionProfile, nullptr),
			InstanceAccessor("isVideoProfile", &RSStreamProfile::IsVideoProfile, nullptr),
			InstanceAccessor("streamType", &RSStreamProfile::Stream, nullptr),
			InstanceAccessor("uniqueId", &RSStreamProfile::UniqueId, nullptr),
			InstanceAccessor("width", &RSStreamProfile::Width, nullptr),
			InstanceMethod("destroy", &RSStreamProfile::Destroy),
			InstanceMethod("getExtrinsicsTo", &RSStreamProfile::GetExtrinsicsTo),
			InstanceMethod("getMotionIntrinsics", &RSStreamProfile::GetMotionIntrinsics),
			InstanceMethod("getVideoStreamIntrinsics", &RSStreamProfile::GetVideoStreamIntrinsics),
		  });
		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSStreamProfile", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_stream_profile* p, bool own = false) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		auto unwrapped			= ObjectWrap<RSStreamProfile>::Unwrap(instance);
		unwrapped->profile_		= p;
		unwrapped->own_profile_ = own;
		CallNativeFunc(
		  rs2_get_stream_profile_data,
		  &unwrapped->error_,
		  p,
		  &unwrapped->stream_,
		  &unwrapped->format_,
		  &unwrapped->index_,
		  &unwrapped->unique_id_,
		  &unwrapped->fps_,
		  &unwrapped->error_);

		unwrapped->is_default_ = rs2_is_stream_profile_default(p, &unwrapped->error_);

		if (GetNativeResult<
			  bool>(rs2_stream_profile_is, &unwrapped->error_, p, RS2_EXTENSION_VIDEO_PROFILE, &unwrapped->error_)) {
			unwrapped->is_video_ = true;
			CallNativeFunc(
			  rs2_get_video_stream_resolution,
			  &unwrapped->error_,
			  p,
			  &unwrapped->width_,
			  &unwrapped->height_,
			  &unwrapped->error_);
		}
		else if (
		  GetNativeResult<
			bool>(rs2_stream_profile_is, &unwrapped->error_, p, RS2_EXTENSION_MOTION_PROFILE, &unwrapped->error_)) {
			unwrapped->is_motion_ = true;
		}

		return scope.Escape(napi_value(instance)).ToObject();
	}

	RSStreamProfile(const CallbackInfo& info)
	  : ObjectWrap<RSStreamProfile>(info)
	  , error_(nullptr)
	  , profile_(nullptr)
	  , index_(0)
	  , unique_id_(0)
	  , fps_(0)
	  , format_(static_cast<rs2_format>(0))
	  , stream_(static_cast<rs2_stream>(0))
	  , is_video_(false)
	  , width_(0)
	  , height_(0)
	  , is_default_(false)
	  , own_profile_(false)
	  , is_motion_(false) {
	}

	~RSStreamProfile() {
		DestroyMe();
	}

  private:
  	friend class RSSensor;

	static FunctionReference constructor;

	rs2_error* error_;
	rs2_stream_profile* profile_;
	int32_t index_;
	int32_t unique_id_;
	int32_t fps_;
	rs2_format format_;
	rs2_stream stream_;
	bool is_video_;
	int32_t width_;
	int32_t height_;
	bool is_default_;
	bool own_profile_;
	bool is_motion_;

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (profile_ && own_profile_) rs2_delete_stream_profile(profile_);
		profile_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.This();
	}

	Napi::Value Format(const CallbackInfo& info) {
		return Number::New(info.Env(), this->format_);
	}

	Napi::Value Fps(const CallbackInfo& info) {
		return Number::New(info.Env(), this->fps_);
	}

	Napi::Value GetExtrinsicsTo(const CallbackInfo& info) {
		auto to = ObjectWrap<RSStreamProfile>::Unwrap(info[0].ToObject());
		if (!to) return info.Env().Undefined();

		rs2_extrinsics res;
		CallNativeFunc(rs2_get_extrinsics, &this->error_, this->profile_, to->profile_, &res, &this->error_);
		if (this->error_) return info.Env().Undefined();

		RSExtrinsics rsres(info.Env(), res);
		return rsres.GetObject();
	}

	Napi::Value GetMotionIntrinsics(const CallbackInfo& info) {
		rs2_motion_device_intrinsic output;
		CallNativeFunc(rs2_get_motion_intrinsics, &this->error_, this->profile_, &output, &this->error_);
		if (this->error_) return info.Env().Undefined();

		RSMotionIntrinsics intrinsics(info.Env(), &output);
		return intrinsics.GetObject();
	}

	Napi::Value GetVideoStreamIntrinsics(const CallbackInfo& info) {
		rs2_intrinsics intr;
		CallNativeFunc(rs2_get_video_stream_intrinsics, &this->error_, this->profile_, &intr, &this->error_);
		if (this->error_) return info.Env().Undefined();

		RSIntrinsics res(info.Env(), intr);
		return res.GetObject();
	}

	Napi::Value Height(const CallbackInfo& info) {
		return Number::New(info.Env(), this->height_);
	}

	Napi::Value Index(const CallbackInfo& info) {
		return Number::New(info.Env(), this->index_);
	}

	Napi::Value IsDefault(const CallbackInfo& info) {
		return Boolean::New(info.Env(), this->is_default_);
	}

	Napi::Value IsMotionProfile(const CallbackInfo& info) {
		return Boolean::New(info.Env(), this->is_motion_);
	}

	Napi::Value IsVideoProfile(const CallbackInfo& info) {
		return Boolean::New(info.Env(), this->is_video_);
	}

	Napi::Value Stream(const CallbackInfo& info) {
		return Number::New(info.Env(), this->stream_);
	}

	Napi::Value UniqueId(const CallbackInfo& info) {
		return Number::New(info.Env(), this->unique_id_);
	}

	Napi::Value Width(const CallbackInfo& info) {
		return Number::New(info.Env(), this->width_);
	}
};

Napi::FunctionReference RSStreamProfile::constructor;

#endif
