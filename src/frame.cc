#ifndef FRAME_H
#define FRAME_H

#include "stream_profile.cc"
#include "utils.cc"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>
using namespace Napi;

class RSFrame : public ObjectWrap<RSFrame> {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSFrame",
		  { InstanceMethod("destroy", &RSFrame::Destroy),
			InstanceMethod("getStreamProfile", &RSFrame::GetStreamProfile),
			InstanceMethod("getData", &RSFrame::GetData),
			InstanceMethod("writeData", &RSFrame::WriteData),
			InstanceMethod("getWidth", &RSFrame::GetWidth),
			InstanceMethod("getHeight", &RSFrame::GetHeight),
			InstanceMethod("getStrideInBytes", &RSFrame::GetStrideInBytes),
			InstanceMethod("getBitsPerPixel", &RSFrame::GetBitsPerPixel),
			InstanceMethod("getTimestamp", &RSFrame::GetTimestamp),
			InstanceMethod("getTimestampDomain", &RSFrame::GetTimestampDomain),
			InstanceMethod("getFrameNumber", &RSFrame::GetFrameNumber),
			InstanceMethod("getFrameMetadata", &RSFrame::GetFrameMetadata),
			InstanceMethod("supportsFrameMetadata", &RSFrame::SupportsFrameMetadata),
			InstanceMethod("isVideoFrame", &RSFrame::IsVideoFrame),
			InstanceMethod("isDepthFrame", &RSFrame::IsDepthFrame),
			InstanceMethod("isDisparityFrame", &RSFrame::IsDisparityFrame),
			InstanceMethod("isMotionFrame", &RSFrame::IsMotionFrame),
			InstanceMethod("isPoseFrame", &RSFrame::IsPoseFrame),
			InstanceMethod("canGetPoints", &RSFrame::CanGetPoints),
			InstanceMethod("getVertices", &RSFrame::GetVertices),
			InstanceMethod("getVerticesBufferLen", &RSFrame::GetVerticesBufferLen),
			InstanceMethod("getTexCoordBufferLen", &RSFrame::GetTexCoordBufferLen),
			InstanceMethod("writeVertices", &RSFrame::WriteVertices),
			InstanceMethod("getTextureCoordinates", &RSFrame::GetTextureCoordinates),
			InstanceMethod("writeTextureCoordinates", &RSFrame::WriteTextureCoordinates),
			InstanceMethod("getPointsCount", &RSFrame::GetPointsCount),
			InstanceMethod("exportToPly", &RSFrame::ExportToPly),
			InstanceMethod("isValid", &RSFrame::IsValid),
			InstanceMethod("getDistance", &RSFrame::GetDistance),
			InstanceMethod("getBaseLine", &RSFrame::GetBaseLine),
			InstanceMethod("keep", &RSFrame::Keep),
			InstanceMethod("getMotionData", &RSFrame::GetMotionData),
			InstanceMethod("getPoseData", &RSFrame::GetPoseData)

		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSFrame", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env, rs2_frame* frame) {
		EscapableHandleScope scope(env);
		Object instance   = constructor.New({});
		auto unwrapped	= ObjectWrap<RSFrame>::Unwrap(instance);
		unwrapped->frame_ = frame;

		return scope.Escape(napi_value(instance)).ToObject();
	}

	void Replace(rs2_frame* value) {
		DestroyMe();
		this->frame_ = value;
		// As the underlying frame changed, we must clean the js side's buffer
		// Function::MakeCallback(this, "_internalResetBuffer", 0, nullptr);
	}

	RSFrame(const CallbackInfo& info)
	  : ObjectWrap<RSFrame>(info)
	  , frame_(nullptr)
	  , error_(nullptr) {
	}

	~RSFrame() {
		DestroyMe();
	}

  private:

	void DestroyMe() {
		if (this->error_) rs2_free_error(error_);
		if (this->frame_) rs2_release_frame(frame_);
		this->error_ = nullptr;
		this->frame_ = nullptr;
	}

	static void SetAFloatInVectorObject(Napi::Env env, Object obj, uint32_t index, float value) {
		const char* names[4] = { "x", "y", "z", "w" };
		obj.Set(names[index], Number::New(env, value));
	}

	static void FillAFloatVector(Napi::Env env, Object obj, const rs2_vector& vec) {
		SetAFloatInVectorObject(env, obj, 0, vec.x);
		SetAFloatInVectorObject(env, obj, 1, vec.y);
		SetAFloatInVectorObject(env, obj, 2, vec.z);
	}

	static void FillAFloatQuaternion(Napi::Env env, Object obj, const rs2_quaternion& quaternion) {
		SetAFloatInVectorObject(env, obj, 0, quaternion.x);
		SetAFloatInVectorObject(env, obj, 1, quaternion.y);
		SetAFloatInVectorObject(env, obj, 2, quaternion.z);
		SetAFloatInVectorObject(env, obj, 3, quaternion.w);
	}

	static void AssemblePoseData(Napi::Env env, Object obj, const rs2_pose& pose) {
		auto translation_name		   = String::New(env, "translation");
		auto velocity_name			   = String::New(env, "velocity");
		auto acceleration_name		   = String::New(env, "acceleration");
		auto rotation_name			   = String::New(env, "rotation");
		auto angular_velocity_name	 = String::New(env, "angularVelocity");
		auto angular_acceleration_name = String::New(env, "angularAcceleration");
		auto tracker_confidence_name   = String::New(env, "trackerConfidence");
		auto mapper_confidence_name	= String::New(env, "mapperConfidence");

		auto translation_obj		  = obj.Get(translation_name).ToObject();
		auto velocity_obj			  = obj.Get(velocity_name).ToObject();
		auto acceleration_obj		  = obj.Get(acceleration_name).ToObject();
		auto rotation_obj			  = obj.Get(rotation_name).ToObject();
		auto angular_velocity_obj	 = obj.Get(angular_velocity_name).ToObject();
		auto angular_acceleration_obj = obj.Get(angular_acceleration_name).ToObject();

		FillAFloatVector(env, translation_obj, pose.translation);
		FillAFloatVector(env, velocity_obj, pose.velocity);
		FillAFloatVector(env, acceleration_obj, pose.acceleration);
		FillAFloatQuaternion(env, rotation_obj, pose.rotation);
		FillAFloatVector(env, angular_velocity_obj, pose.angular_velocity);
		FillAFloatVector(env, angular_acceleration_obj, pose.angular_acceleration);

		obj.Set(tracker_confidence_name, Number::New(env, pose.tracker_confidence));
		obj.Set(mapper_confidence_name, Number::New(env, pose.mapper_confidence));
	}

	Napi::Value GetStreamProfile(const CallbackInfo& info) {
		rs2_stream stream;
		rs2_format format;
		int32_t index						  = 0;
		int32_t unique_id					  = 0;
		int32_t fps							  = 0;
		const rs2_stream_profile* profile_org = GetNativeResult<
		  const rs2_stream_profile*>(rs2_get_frame_stream_profile, &this->error_, this->frame_, &this->error_);
		CallNativeFunc(
		  rs2_get_stream_profile_data,
		  &this->error_,
		  profile_org,
		  &stream,
		  &format,
		  &index,
		  &unique_id,
		  &fps,
		  &this->error_);
		if (this->error_) return info.Env().Undefined();

		rs2_stream_profile* profile = GetNativeResult<
		  rs2_stream_profile*>(rs2_clone_stream_profile, &this->error_, profile_org, stream, index, format, &this->error_);
		if (!profile) return info.Env().Undefined();

		return RSStreamProfile::NewInstance(info.Env(), profile, true);
	}

	Napi::Value GetData(const CallbackInfo& info) {
		auto buffer = GetNativeResult<const void*>(rs2_get_frame_data, &this->error_, this->frame_, &this->error_);
		if (!buffer) return info.Env().Undefined();

		const auto stride
		  = GetNativeResult<int>(rs2_get_frame_stride_in_bytes, &this->error_, this->frame_, &this->error_);
		const auto height = GetNativeResult<int>(rs2_get_frame_height, &this->error_, this->frame_, &this->error_);
		const auto length = stride * height;
		auto array_buffer = ArrayBuffer::New(info.Env(), static_cast<uint8_t*>(const_cast<void*>(buffer)), length);
		return TypedArrayOf<uint8_t>::New(info.Env(), length, array_buffer, 0);
	}

	Napi::Value WriteData(const CallbackInfo& info) {
		auto array_buffer = info[0].As<ArrayBuffer>();

		const auto buffer
		  = GetNativeResult<const void*>(rs2_get_frame_data, &this->error_, this->frame_, &this->error_);
		const auto stride
		  = GetNativeResult<int>(rs2_get_frame_stride_in_bytes, &this->error_, this->frame_, &this->error_);
		const auto height   = GetNativeResult<int>(rs2_get_frame_height, &this->error_, this->frame_, &this->error_);
		const size_t length = stride * height;
		if (buffer && array_buffer.ByteLength() >= length) { memcpy(array_buffer.Data(), buffer, length); }
		return info.Env().Undefined();
	}

	Napi::Value GetWidth(const CallbackInfo& info) {
		auto value = GetNativeResult<int>(rs2_get_frame_width, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), value);
	}

	Napi::Value GetHeight(const CallbackInfo& info) {
		auto value = GetNativeResult<int>(rs2_get_frame_height, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), value);
	}

	Napi::Value GetStrideInBytes(const CallbackInfo& info) {
		auto value = GetNativeResult<int>(rs2_get_frame_stride_in_bytes, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), value);
	}

	Napi::Value GetBitsPerPixel(const CallbackInfo& info) {
		auto value = GetNativeResult<int>(rs2_get_frame_bits_per_pixel, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), value);
	}

	Napi::Value GetTimestamp(const CallbackInfo& info) {
		auto value = GetNativeResult<double>(rs2_get_frame_timestamp, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), value);
	}

	Napi::Value GetTimestampDomain(const CallbackInfo& info) {
		auto value = GetNativeResult<
		  rs2_timestamp_domain>(rs2_get_frame_timestamp_domain, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), value);
	}

	Napi::Value GetFrameNumber(const CallbackInfo& info) {
		uint32_t value = GetNativeResult<uint32_t>(rs2_get_frame_number, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), value);
	}

	Napi::Value IsVideoFrame(const CallbackInfo& info) {
		bool isVideo = false;
		if (GetNativeResult<
			  bool>(rs2_is_frame_extendable_to, &this->error_, this->frame_, RS2_EXTENSION_VIDEO_FRAME, &this->error_))
			isVideo = true;
		return Boolean::New(info.Env(), isVideo);
	}

	Napi::Value IsDepthFrame(const CallbackInfo& info) {
		bool isDepth = false;
		if (GetNativeResult<
			  bool>(rs2_is_frame_extendable_to, &this->error_, this->frame_, RS2_EXTENSION_DEPTH_FRAME, &this->error_))
			isDepth = true;
		return Boolean::New(info.Env(), isDepth);
	}

	Napi::Value IsDisparityFrame(const CallbackInfo& info) {
		auto is_disparity = GetNativeResult<
		  int>(rs2_is_frame_extendable_to, &this->error_, this->frame_, RS2_EXTENSION_DISPARITY_FRAME, &this->error_);
		return Boolean::New(info.Env(), is_disparity ? true : false);
	}

	Napi::Value IsMotionFrame(const CallbackInfo& info) {
		auto val = GetNativeResult<
		  int>(rs2_is_frame_extendable_to, &this->error_, this->frame_, RS2_EXTENSION_MOTION_FRAME, &this->error_);
		return Boolean::New(info.Env(), val ? true : false);
	}

	Napi::Value IsPoseFrame(const CallbackInfo& info) {
		auto val = GetNativeResult<
		  int>(rs2_is_frame_extendable_to, &this->error_, this->frame_, RS2_EXTENSION_POSE_FRAME, &this->error_);
		return Boolean::New(info.Env(), val ? true : false);
	}

	Napi::Value GetFrameMetadata(const CallbackInfo& info) {
		rs2_frame_metadata_value metadata = static_cast<rs2_frame_metadata_value>(info[0].ToNumber().Int32Value());
		TypedArray content(info.Env(), info[1]);
		auto data = content.ArrayBuffer().Data();
		if (!data) return Boolean::New(info.Env(), false);

		rs2_metadata_type output = GetNativeResult<
		  rs2_metadata_type>(rs2_get_frame_metadata, &this->error_, this->frame_, metadata, &this->error_);
		unsigned char* out_ptr = reinterpret_cast<unsigned char*>(&output);
		uint32_t val		   = 1;
		unsigned char* val_ptr = reinterpret_cast<unsigned char*>(&val);

		if (*val_ptr == 0) {
			// big endian
			memcpy(data, out_ptr, 8);
		}
		else {
			// little endian
			for (int32_t i = 0; i < 8; i++) { (&data)[i] = out_ptr[7 - i]; }
		}

		return Boolean::New(info.Env(), true);
	}

	Napi::Value SupportsFrameMetadata(const CallbackInfo& info) {
		rs2_frame_metadata_value metadata = (rs2_frame_metadata_value)(info[0].ToNumber().Int32Value());

		int result
		  = GetNativeResult<int>(rs2_supports_frame_metadata, &this->error_, this->frame_, metadata, &this->error_);
		return Boolean::New(info.Env(), result ? true : false);
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	Napi::Value CanGetPoints(const CallbackInfo& info) {
		bool result = false;
		if (GetNativeResult<
			  int>(rs2_is_frame_extendable_to, &this->error_, this->frame_, RS2_EXTENSION_POINTS, &this->error_))
			result = true;
		return Boolean::New(info.Env(), result);
	}

	Napi::Value GetVertices(const CallbackInfo& info) {
		rs2_vertex* vertices
		  = GetNativeResult<rs2_vertex*>(rs2_get_frame_vertices, &this->error_, this->frame_, &this->error_);
		size_t count = GetNativeResult<size_t>(rs2_get_frame_points_count, &this->error_, this->frame_, &this->error_);
		if (!vertices || !count) return info.Env().Undefined();

		uint32_t step   = 3 * sizeof(float);
		uint32_t len	= count * step;
		auto vertex_buf = static_cast<uint8_t*>(malloc(len));

		for (size_t i = 0; i < count; i++) { memcpy(vertex_buf + i * step, vertices[i].xyz, step); }
		auto array_buffer = ArrayBuffer::New(info.Env(), vertex_buf, len);
		return TypedArrayOf<float>::New(info.Env(), 3 * count, array_buffer, 0);
	}

	Napi::Value GetVerticesBufferLen(const CallbackInfo& info) {
		const size_t count
		  = GetNativeResult<size_t>(rs2_get_frame_points_count, &this->error_, this->frame_, &this->error_);
		const uint32_t step   = 3 * sizeof(float);
		const uint32_t length = count * step;
		return Number::New(info.Env(), length);
	}

	Napi::Value GetTexCoordBufferLen(const CallbackInfo& info) {
		const size_t count
		  = GetNativeResult<size_t>(rs2_get_frame_points_count, &this->error_, this->frame_, &this->error_);
		const uint32_t step   = 2 * sizeof(int);
		const uint32_t length = count * step;
		return Number::New(info.Env(), length);
	}

	Napi::Value WriteVertices(const CallbackInfo& info) {
		auto array_buffer = info[0].As<ArrayBuffer>();

		const rs2_vertex* vertBuf
		  = GetNativeResult<rs2_vertex*>(rs2_get_frame_vertices, &this->error_, this->frame_, &this->error_);
		const size_t count
		  = GetNativeResult<size_t>(rs2_get_frame_points_count, &this->error_, this->frame_, &this->error_);
		if (!vertBuf || !count) return Boolean::New(info.Env(), false);

		const uint32_t step   = 3 * sizeof(float);
		const uint32_t length = count * step;
		if (array_buffer.ByteLength() < length) return Boolean::New(info.Env(), false);

		uint8_t* vertex_buf = static_cast<uint8_t*>(array_buffer.Data());
		for (size_t i = 0; i < count; i++) { memcpy(vertex_buf + i * step, vertBuf[i].xyz, step); }
		return Boolean::New(info.Env(), true);
	}

	Napi::Value GetTextureCoordinates(const CallbackInfo& info) {
		rs2_pixel* coords
		  = GetNativeResult<rs2_pixel*>(rs2_get_frame_texture_coordinates, &this->error_, this->frame_, &this->error_);
		size_t count = GetNativeResult<size_t>(rs2_get_frame_points_count, &this->error_, this->frame_, &this->error_);
		if (!coords || !count) return info.Env().Undefined();

		uint32_t step	 = 2 * sizeof(int);
		uint32_t len	  = count * step;
		auto texcoord_buf = static_cast<uint8_t*>(malloc(len));

		for (size_t i = 0; i < count; ++i) { memcpy(texcoord_buf + i * step, coords[i].ij, step); }
		auto array_buffer = ArrayBuffer::New(info.Env(), texcoord_buf, len);

		return TypedArrayOf<float>::New(info.Env(), 2 * count, array_buffer, 0);
	}

	Napi::Value WriteTextureCoordinates(const CallbackInfo& info) {
		auto array_buffer = info[0].As<ArrayBuffer>();

		const rs2_pixel* coords = rs2_get_frame_texture_coordinates(this->frame_, &this->error_);
		const size_t count
		  = GetNativeResult<size_t>(rs2_get_frame_points_count, &this->error_, this->frame_, &this->error_);
		if (!coords || !count) return Boolean::New(info.Env(), false);

		const uint32_t step   = 2 * sizeof(int);
		const uint32_t length = count * step;
		if (array_buffer.ByteLength() < length) return Boolean::New(info.Env(), false);

		uint8_t* texcoord_buf = static_cast<uint8_t*>(array_buffer.Data());
		for (size_t i = 0; i < count; ++i) { memcpy(texcoord_buf + i * step, coords[i].ij, step); }
		return Boolean::New(info.Env(), true);
	}

	Napi::Value GetPointsCount(const CallbackInfo& info) {
		int32_t count = GetNativeResult<int>(rs2_get_frame_points_count, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), count);
	}

	Napi::Value ExportToPly(const CallbackInfo& info) {
		auto texture = ObjectWrap<RSFrame>::Unwrap(info[1].ToObject());
		auto file	= std::string(info[0].ToString()).c_str();
		if (!texture) return info.Env().Undefined();

		rs2_frame* ptr = nullptr;
		std::swap(texture->frame_, ptr);
		CallNativeFunc(rs2_export_to_ply, &this->error_, this->frame_, file, ptr, &this->error_);
		return info.Env().Undefined();
	}

	Napi::Value IsValid(const CallbackInfo& info) {
		return Boolean::New(info.Env(), this->frame_ ? true : false);
	}

	Napi::Value GetDistance(const CallbackInfo& info) {
		auto x = info[0].ToNumber().Int32Value();
		auto y = info[1].ToNumber().Int32Value();

		auto val
		  = GetNativeResult<float>(rs2_depth_frame_get_distance, &this->error_, this->frame_, x, y, &this->error_);
		return Number::New(info.Env(), val);
	}

	Napi::Value GetBaseLine(const CallbackInfo& info) {
		auto val
		  = GetNativeResult<float>(rs2_depth_stereo_frame_get_baseline, &this->error_, this->frame_, &this->error_);
		return Number::New(info.Env(), val);
	}

	Napi::Value Keep(const CallbackInfo& info) {
		if (!this->frame_) return info.Env().Undefined();

		rs2_keep_frame(this->frame_);
		return info.Env().Undefined();
	}

	Napi::Value GetMotionData(const CallbackInfo& info) {
		auto obj		= info[0].ToObject();
		auto frame_data = static_cast<const float*>(
		  GetNativeResult<const void*>(rs2_get_frame_data, &this->error_, this->frame_, &this->error_));
		for (uint32_t i = 0; i < 3; i++) { SetAFloatInVectorObject(info.Env(), obj, i, frame_data[i]); }
		return info.Env().Undefined();
	}

	Napi::Value GetPoseData(const CallbackInfo& info) {
		rs2_pose pose_data;
		CallNativeFunc(rs2_pose_frame_get_pose_data, &this->error_, this->frame_, &pose_data, &this->error_);
		if (this->error_) return Boolean::New(info.Env(), false);

		AssemblePoseData(info.Env(), info[0].ToObject(), pose_data);
		return Boolean::New(info.Env(), true);
	}

  private:
	static FunctionReference constructor;
	rs2_frame* frame_;
	rs2_error* error_;
	friend class RSColorizer;
	friend class RSFilter;
	friend class RSFrameQueue;
	friend class RSPointCloud;
	friend class RSSyncer;
};

Napi::FunctionReference RSFrame::constructor;

#endif
