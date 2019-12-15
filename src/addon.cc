#include "config.cc"
#include "context.cc"
#include "align.cc"
#include "device.cc"
#include "device_hub.cc"
#include "device_list.cc"
#include "devices_changed_callback.cc"
#include "frame.cc"
#include "frameset.cc"
#include "colorizer.cc"
#include "pipeline.cc"
#include "pipeline_profile.cc"
#include "sensor.cc"
#include "stream_profile.cc"
#include "syncer.cc"
#include <librealsense2/h/rs_internal.h>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/hpp/rs_types.hpp>
#include <librealsense2/rs.h>
#include <napi.h>

// using namespace std;
using namespace Napi;

Value GetError(const CallbackInfo& info) {
	return ErrorUtil::GetJSErrorObject(info.Env());
}

Value GetTime(const CallbackInfo& info) {
	rs2_error* e = nullptr;
	auto time	= rs2_get_time(&e);
	return Number::New(info.Env(), time);
}

Value RegisterErrorCallback(const CallbackInfo& info) {
	ErrorUtil::Init(info.Env());
	ErrorUtil::UpdateJSErrorCallback(info);

	return info.Env().Undefined();
}

Value Cleanup(const CallbackInfo& info) {
	// MainThreadCallback::Destroy();
	ErrorUtil::ResetError();

	return info.Env().Undefined();
}

Object Init(Env env, Object exports) {
	exports.Set("cleanup", Function::New(env, Cleanup));
	exports.Set("getError", Function::New(env, GetError));
	exports.Set("getTime", Function::New(env, GetTime));
	exports.Set("registerErrorCallback", Function::New(env, RegisterErrorCallback));

	// RSFilter::Init(env, exports);
	// RSFrameQueue::Init(env, exports);
	// RSPointCloud::Init(env, exports);
	RSAlign::Init(env, exports);
	RSColorizer::Init(env, exports);
	RSConfig::Init(env, exports);
	RSContext::Init(env, exports);
	RSDevice::Init(env, exports);
	RSDeviceHub::Init(env, exports);
	RSDeviceList::Init(env, exports);
	RSFrame::Init(env, exports);
	RSFrameSet::Init(env, exports);
	RSPipeline::Init(env, exports);
	RSPipelineProfile::Init(env, exports);
	RSSensor::Init(env, exports);
	RSStreamProfile::Init(env, exports);
	RSSyncer::Init(env, exports);

	return exports;
}

NODE_API_MODULE(realsense_node, Init)
