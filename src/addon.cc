#include <librealsense2/h/rs_internal.h>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/hpp/rs_types.hpp>
#include <librealsense2/rs.h>

#include <napi.h>

#include "dict_base.cc"
#include "error_util.cc"
#include "main_thread_callback.cc"

// using namespace std;
using namespace Napi;

Value GetError(const CallbackInfo& info) {
	return ErrorUtil::GetJSErrorObject(info.Env());
}

Value GetTime(const CallbackInfo& info) {
	rs2_error* e = nullptr;
	auto time = rs2_get_time(&e);
	return Number::New(info.Env(), time);
}

Value RegisterErrorCallback(const CallbackInfo& info) {
  ErrorUtil::Init(info.Env());
  ErrorUtil::UpdateJSErrorCallback(info);

  return info.Env().Undefined();
}


Object Init(Env env, Object exports) {
	exports.Set("getError", Function::New(env, GetError));
	exports.Set("getTime", Function::New(env, GetTime));
	exports.Set("registerErrorCallback", Function::New(env, RegisterErrorCallback));
	return exports;
}

NODE_API_MODULE(realsense_node, Init)
