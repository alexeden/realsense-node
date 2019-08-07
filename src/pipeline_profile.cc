#ifndef PIPELINE_PROFILE_H
#define PIPELINE_PROFILE_H

#include "config.cc"
#include "device.cc"
#include "utils.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSPipelineProfile : public ObjectWrap<RSPipelineProfile> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSPipelineProfile",
		  {
			InstanceMethod("getStreams", &RSPipelineProfile::GetStreams),
			InstanceMethod("getDevice", &RSPipelineProfile::GetDevice),
			InstanceMethod("destroy", &RSPipelineProfile::Destroy),
		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSPipelineProfile", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_pipeline_profile* profile) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		auto unwrapped				 = ObjectWrap<RSPipelineProfile>::Unwrap(instance);
		unwrapped->pipeline_profile_ = profile;

		return scope.Escape(napi_value(instance)).ToObject();
	}

	RSPipelineProfile(const CallbackInfo& info)
	  : ObjectWrap<RSPipelineProfile>(info)
	  , pipeline_profile_(nullptr)
	  , error_(nullptr) {
	}

	~RSPipelineProfile() {
		DestroyMe();
	}

  private:
	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;

		if (pipeline_profile_) rs2_delete_pipeline_profile(pipeline_profile_);
		pipeline_profile_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	Napi::Value GetStreams(const CallbackInfo& info) {
		rs2_stream_profile_list* list = GetNativeResult<
		  rs2_stream_profile_list*>(rs2_pipeline_profile_get_streams, &this->error_, this->pipeline_profile_, &this->error_);
		if (!list) return info.Env().Undefined();

		int32_t size = GetNativeResult<int32_t>(rs2_get_stream_profiles_count, &this->error_, list, &this->error_);
		if (this->error_) return info.Env().Undefined();

		auto array = Array::New(info.Env(), size);
		for (int32_t i = 0; i < size; i++) {
			rs2_stream_profile* profile = const_cast<rs2_stream_profile*>(
			  GetNativeResult<
				const rs2_stream_profile*>(rs2_get_stream_profile, &this->error_, list, i, &this->error_));
			array.Set(i, RSStreamProfile::NewInstance(info.Env(), profile));
		}

		return array;
	}

	Napi::Value GetDevice(const CallbackInfo& info) {
		rs2_device* dev = GetNativeResult<
		  rs2_device*>(rs2_pipeline_profile_get_device, &this->error_, this->pipeline_profile_, &this->error_);
		if (!dev) return info.Env().Undefined();

		return RSDevice::NewInstance(info.Env(), dev);
	}

  private:
	static FunctionReference constructor;

	rs2_pipeline_profile* pipeline_profile_;
	rs2_error* error_;
};

Napi::FunctionReference RSPipelineProfile::constructor;

#endif
