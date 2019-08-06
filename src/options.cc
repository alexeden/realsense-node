#ifndef OPTIONS_H
#define OPTIONS_H

#include <napi.h>
#include <librealsense2/hpp/rs_types.hpp>
#include "utils.cc"
#include "dict_base.cc"

using namespace Napi;

class RSOptionRange : public DictBase {
 public:
  RSOptionRange(Env env, float min, float max, float step, float def) : DictBase(env) {
    SetMemberT("minValue", min);
    SetMemberT("maxValue", max);
    SetMemberT("step", step);
    SetMemberT("defaultValue", def);
  }
};



class Options {
  public:
	Options()
	  : error_(nullptr) {
	}

	virtual ~Options() {
		if (error_) rs2_free_error(error_);
	}

	virtual rs2_options* GetOptionsPointer() = 0;

	Napi::Value SupportsOptionInternal(const CallbackInfo& info) {
		int32_t option = info[0].ToNumber().Int32Value();
		auto on		   = GetNativeResult<
			 int>(rs2_supports_option, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);

		return Boolean::New(info.Env(), on ? true : false);
	}

	Napi::Value GetOptionInternal(const CallbackInfo& info) {
		int32_t option = info[0].ToNumber().Int32Value();
		auto value	 = GetNativeResult<
		  float>(rs2_get_option, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);

		return Number::New(info.Env(), value);
	}

	Napi::Value GetOptionDescriptionInternal(const CallbackInfo& info) {
		int32_t option = info[0].ToNumber().Int32Value();
		auto desc	  = GetNativeResult<
		   const char*>(rs2_get_option_description, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);

		if (desc)
			return String::New(info.Env(), desc);
		else
			return info.Env().Undefined();
	}

	Napi::Value GetOptionValueDescriptionInternal(const CallbackInfo& info) {
		int32_t option = info[0].ToNumber().Int32Value();
		auto val	   = info[1].ToNumber().Int32Value();
		auto desc	  = GetNativeResult<
		   const char*>(rs2_get_option_value_description, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), val, &error_);
		if (desc)
			return String::New(info.Env(), desc);
		else
			return info.Env().Undefined();
	}

	Napi::Value SetOptionInternal(const CallbackInfo& info) {
		int32_t option = info[0].ToNumber().Int32Value();
		auto val	   = info[1].ToNumber().Int32Value();
		CallNativeFunc(rs2_set_option, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), val, &error_);
		return info.Env().Undefined();
	}

	Napi::Value GetOptionRangeInternal(const CallbackInfo& info) {
		int32_t option = info[0].ToNumber().Int32Value();
		float min	  = 0;
		float max	  = 0;
		float step	 = 0;
		float def	  = 0;
		CallNativeFunc(
		  rs2_get_option_range,
		  &error_,
		  GetOptionsPointer(),
		  static_cast<rs2_option>(option),
		  &min,
		  &max,
		  &step,
		  &def,
		  &error_);

		return RSOptionRange(info.Env(), min, max, step, def).GetObject();
	}

	Napi::Value IsOptionReadonlyInternal(const CallbackInfo& info) {
		int32_t option = info[0].ToNumber().Int32Value();
		auto val	   = GetNativeResult<
			int>(rs2_is_option_read_only, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);

		return Boolean::New(info.Env(), val ? true : false);
	}

  private:
	rs2_error* error_;
};

#endif
