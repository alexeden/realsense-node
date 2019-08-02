#include <librealsense2/h/rs_internal.h>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/hpp/rs_types.hpp>
#include <librealsense2/rs.h>

#include <napi.h>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	return exports;
}

NODE_API_MODULE(realsense_node, Init)
