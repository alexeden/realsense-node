#ifndef SENSOR_H
#define SENSOR_H

#include "dict_base.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSExtrinsics : public DictBase {
  public:
	RSExtrinsics(Napi::Env env, rs2_extrinsics extrinsics)
	  : DictBase(env) {
		SetMemberArray<Number, float, 9>("rotation", extrinsics.rotation);
		SetMemberArray<Number, float, 3>("translation", extrinsics.translation);
	}
};

class RSIntrinsics : public DictBase {
  public:
	explicit RSIntrinsics(Napi::Env env, rs2_intrinsics intrinsics)
	  : DictBase(env) {
		SetMemberT("width", intrinsics.width);
		SetMemberT("height", intrinsics.height);
		SetMemberT("ppx", intrinsics.ppx);
		SetMemberT("ppy", intrinsics.ppy);
		SetMemberT("fx", intrinsics.fx);
		SetMemberT("fy", intrinsics.fy);
		SetMemberT("model", intrinsics.model);
		SetMemberArray<Number, float, 5>("coeffs", intrinsics.coeffs);
	}
};

class RSMotionIntrinsics : public DictBase {
  public:
	RSMotionIntrinsics(Napi::Env env, rs2_motion_device_intrinsic* intri)
	  : DictBase(env) {
		auto data_array = Array::New(env, 3);
		int32_t index   = 0;
		for (int32_t i = 0; i < 3; i++) {
			for (int32_t j = 0; j < 4; j++) { data_array.Set(index++, Number::New(env, intri->data[i][j])); }
		}
		SetMember("data", data_array);
		SetMemberArray<Number, float, 3>("noiseVariances", intri->noise_variances);
		SetMemberArray<Number, float, 3>("biasVariances", intri->bias_variances);
	}
};

class RSNotification : public DictBase {
  public:
	RSNotification(
	  Napi::Env env,
	  const std::string& des,
	  rs2_time_t time,
	  rs2_log_severity severity,
	  rs2_notification_category category,
	  const std::string& serialized_data)
	  : DictBase(env) {
		SetMember("descr", des);
		SetMemberT("timestamp", time);
		SetMemberT("severity", (int32_t) severity);
		SetMemberT("category", (int32_t) category);
		SetMember("serializedData", serialized_data);
	}
};

class RSOptionRange : public DictBase {
  public:
	RSOptionRange(Napi::Env env, float min, float max, float step, float def)
	  : DictBase(env) {
		SetMemberT("minValue", min);
		SetMemberT("maxValue", max);
		SetMemberT("step", step);
		SetMemberT("defaultValue", def);
	}
};

class RSRegionOfInterest : public DictBase {
  public:
	RSRegionOfInterest(Napi::Env env, int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
	  : DictBase(env) {
		SetMemberT("minX", minx);
		SetMemberT("minY", miny);
		SetMemberT("maxX", maxx);
		SetMemberT("maxY", maxy);
	}
};

#endif
