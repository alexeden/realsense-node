#include <librealsense2/h/rs_internal.h>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/hpp/rs_types.hpp>
#include <librealsense2/rs.h>

#include <napi.h>

using namespace std;
using namespace Napi;

Value GetTime(const CallbackInfo& info) {
	fprintf(stderr, "Getting time");
	rs2_error* e = nullptr;
	fprintf(stderr, "Got e");
	auto time = rs2_get_time(&e);
	fprintf(stderr, "Got time!");
	return Number::New(info.Env(), time);
}

Object Init(Env env, Object exports) {
	exports.Set("getTime", Function::New(env, GetTime));

	return exports;
}

NODE_API_MODULE(realsense_node, Init)
