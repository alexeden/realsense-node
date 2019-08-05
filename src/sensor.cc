#ifndef SENSOR_H
#define SENSOR_H

class RSSensor
  : public Nan::ObjectWrap
  , Options {
  public:
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSSensor").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "openStream", OpenStream);
		Nan::SetPrototypeMethod(tpl, "openMultipleStream", OpenMultipleStream);
		Nan::SetPrototypeMethod(tpl, "getCameraInfo", GetCameraInfo);
		Nan::SetPrototypeMethod(tpl, "startWithSyncer", StartWithSyncer);
		Nan::SetPrototypeMethod(tpl, "startWithCallback", StartWithCallback);
		Nan::SetPrototypeMethod(tpl, "supportsOption", SupportsOption);
		Nan::SetPrototypeMethod(tpl, "getOption", GetOption);
		Nan::SetPrototypeMethod(tpl, "setOption", SetOption);
		Nan::SetPrototypeMethod(tpl, "getOptionRange", GetOptionRange);
		Nan::SetPrototypeMethod(tpl, "isOptionReadonly", IsOptionReadonly);
		Nan::SetPrototypeMethod(tpl, "getOptionDescription", GetOptionDescription);
		Nan::SetPrototypeMethod(tpl, "getOptionValueDescription", GetOptionValueDescription);
		Nan::SetPrototypeMethod(tpl, "stop", Stop);
		Nan::SetPrototypeMethod(tpl, "supportsCameraInfo", SupportsCameraInfo);
		Nan::SetPrototypeMethod(tpl, "getStreamProfiles", GetStreamProfiles);
		Nan::SetPrototypeMethod(tpl, "close", Close);
		Nan::SetPrototypeMethod(tpl, "setNotificationCallback", SetNotificationCallback);
		Nan::SetPrototypeMethod(tpl, "setRegionOfInterest", SetRegionOfInterest);
		Nan::SetPrototypeMethod(tpl, "getRegionOfInterest", GetRegionOfInterest);
		Nan::SetPrototypeMethod(tpl, "getDepthScale", GetDepthScale);
		Nan::SetPrototypeMethod(tpl, "isDepthSensor", IsDepthSensor);
		Nan::SetPrototypeMethod(tpl, "isROISensor", IsROISensor);
		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSSensor").ToLocalChecked(), tpl->GetFunction());
	}

	static v8::Local<v8::Object> NewInstance(rs2_sensor* sensor) {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor_);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();

		auto me		= Nan::ObjectWrap::Unwrap<RSSensor>(instance);
		me->sensor_ = sensor;
		return scope.Escape(instance);
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

  private:
	RSSensor()
	  : sensor_(nullptr)
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

	void RegisterNotificationCallbackMethod();

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (sensor_) rs2_delete_sensor(sensor_);
		sensor_ = nullptr;
		if (profile_list_) rs2_delete_stream_profiles_list(profile_list_);
		profile_list_ = nullptr;
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (info.IsConstructCall()) {
			RSSensor* obj = new RSSensor();
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	static NAN_METHOD(SupportsOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return me->SupportsOptionInternal(info);

		info.GetReturnValue().Set(Nan::False());
	}

	static NAN_METHOD(GetOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return me->GetOptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionDescription) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return me->GetOptionDescriptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionValueDescription) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return me->GetOptionValueDescriptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(SetOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return me->SetOptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionRange) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return me->GetOptionRangeInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(IsOptionReadonly) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) return me->IsOptionReadonlyInternal(info);

		info.GetReturnValue().Set(Nan::False());
	}

	static NAN_METHOD(GetCameraInfo) {
		info.GetReturnValue().Set(Nan::Undefined());
		int32_t camera_info = info[0]->IntegerValue();
		;
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		auto value = GetNativeResult<
		  const char*>(rs2_get_sensor_info, &me->error_, me->sensor_, static_cast<rs2_camera_info>(camera_info), &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(value).ToLocalChecked());
	}

	static NAN_METHOD(StartWithSyncer) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto syncer = Nan::ObjectWrap::Unwrap<RSSyncer>(info[0]->ToObject());
		auto me		= Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me || !syncer) return;

		CallNativeFunc(rs2_start_cpp, &me->error_, me->sensor_, new FrameCallbackForProcessingBlock(syncer->syncer_), &me->error_);
	}

	static NAN_METHOD(StartWithCallback) {
		auto frame			 = Nan::ObjectWrap::Unwrap<RSFrame>(info[1]->ToObject());
		auto depth_frame	 = Nan::ObjectWrap::Unwrap<RSFrame>(info[2]->ToObject());
		auto video_frame	 = Nan::ObjectWrap::Unwrap<RSFrame>(info[3]->ToObject());
		auto disparity_frame = Nan::ObjectWrap::Unwrap<RSFrame>(info[4]->ToObject());
		auto motion_frame	= Nan::ObjectWrap::Unwrap<RSFrame>(info[5]->ToObject());
		auto pose_frame		 = Nan::ObjectWrap::Unwrap<RSFrame>(info[6]->ToObject());
		auto me				 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me && frame && depth_frame && video_frame && disparity_frame && motion_frame && pose_frame) {
			me->frame_			 = frame;
			me->depth_frame_	 = depth_frame;
			me->video_frame_	 = video_frame;
			me->disparity_frame_ = disparity_frame;
			me->motion_frame_	= motion_frame;
			me->pose_frame_		 = pose_frame;
			v8::String::Utf8Value str(info[0]);
			me->frame_callback_name_ = std::string(*str);
			CallNativeFunc(rs2_start_cpp, &me->error_, me->sensor_, new FrameCallbackForProc(me), &me->error_);
		}
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (me) { me->DestroyMe(); }
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(OpenStream) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me		 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		auto profile = Nan::ObjectWrap::Unwrap<RSStreamProfile>(info[0]->ToObject());
		if (!me || !profile) return;

		CallNativeFunc(rs2_open, &me->error_, me->sensor_, profile->profile_, &me->error_);
	}

	static NAN_METHOD(OpenMultipleStream) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		auto array   = v8::Local<v8::Array>::Cast(info[0]);
		uint32_t len = array->Length();
		std::vector<const rs2_stream_profile*> profs;
		for (uint32_t i = 0; i < len; i++) {
			auto profile = Nan::ObjectWrap::Unwrap<RSStreamProfile>(array->Get(i)->ToObject());
			profs.push_back(profile->profile_);
		}
		CallNativeFunc(rs2_open_multiple, &me->error_, me->sensor_, profs.data(), len, &me->error_);
	}

	static NAN_METHOD(Stop) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_stop, &me->error_, me->sensor_, &me->error_);
	}

	static NAN_METHOD(GetStreamProfiles) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		rs2_stream_profile_list* list = me->profile_list_;
		if (!list) {
			list = GetNativeResult<
			  rs2_stream_profile_list*>(rs2_get_stream_profiles, &me->error_, me->sensor_, &me->error_);
			me->profile_list_ = list;
		}
		if (!list) return;

		int32_t size = GetNativeResult<int>(rs2_get_stream_profiles_count, &me->error_, list, &me->error_);
		v8::Local<v8::Array> array = Nan::New<v8::Array>(size);
		for (int32_t i = 0; i < size; i++) {
			rs2_stream_profile* profile = const_cast<rs2_stream_profile*>(rs2_get_stream_profile(list, i, &me->error_));
			array->Set(i, RSStreamProfile::NewInstance(profile));
		}
		info.GetReturnValue().Set(array);
	}

	static NAN_METHOD(SupportsCameraInfo) {
		info.GetReturnValue().Set(Nan::False());
		int32_t camera_info = info[0]->IntegerValue();
		auto me				= Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		int32_t on = GetNativeResult<
		  int>(rs2_supports_sensor_info, &me->error_, me->sensor_, (rs2_camera_info) camera_info, &me->error_);
		info.GetReturnValue().Set(Nan::New(on ? true : false));
	}

	static NAN_METHOD(Close) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_close, &me->error_, me->sensor_, &me->error_);
	}

	static NAN_METHOD(SetNotificationCallback) {
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		v8::String::Utf8Value value(info[0]->ToString());
		me->notification_callback_name_ = std::string(*value);
		me->RegisterNotificationCallbackMethod();
	}

	static NAN_METHOD(SetRegionOfInterest) {
		info.GetReturnValue().Set(Nan::Undefined());
		int32_t minx = info[0]->IntegerValue();
		int32_t miny = info[1]->IntegerValue();
		int32_t maxx = info[2]->IntegerValue();
		int32_t maxy = info[3]->IntegerValue();
		auto me		 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_set_region_of_interest, &me->error_, me->sensor_, minx, miny, maxx, maxy, &me->error_);
	}

	static NAN_METHOD(GetRegionOfInterest) {
		info.GetReturnValue().Set(Nan::Undefined());
		int32_t minx = 0;
		int32_t miny = 0;
		int32_t maxx = 0;
		int32_t maxy = 0;
		auto me		 = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		CallNativeFunc(rs2_get_region_of_interest, &me->error_, me->sensor_, &minx, &miny, &maxx, &maxy, &me->error_);
		if (me->error_) return;
		info.GetReturnValue().Set(RSRegionOfInterest(minx, miny, maxx, maxy).GetObject());
	}

	static NAN_METHOD(GetDepthScale) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		auto scale = GetNativeResult<float>(rs2_get_depth_scale, &me->error_, me->sensor_, &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(scale));
	}

	static NAN_METHOD(IsDepthSensor) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		bool is_depth = GetNativeResult<
		  int>(rs2_is_sensor_extendable_to, &me->error_, me->sensor_, RS2_EXTENSION_DEPTH_SENSOR, &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(is_depth));
	}

	static NAN_METHOD(IsROISensor) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSSensor>(info.Holder());
		if (!me) return;

		bool is_roi
		  = GetNativeResult<int>(rs2_is_sensor_extendable_to, &me->error_, me->sensor_, RS2_EXTENSION_ROI, &me->error_);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(is_roi));
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;
	rs2_sensor* sensor_;
	rs2_error* error_;
	rs2_stream_profile_list* profile_list_;
	std::string frame_callback_name_;
	std::string notification_callback_name_;
	RSFrame* frame_;
	RSFrame* video_frame_;
	RSFrame* depth_frame_;
	RSFrame* disparity_frame_;
	RSFrame* motion_frame_;
	RSFrame* pose_frame_;
	friend class RSContext;
	friend class DevicesChangedCallbackInfo;
	friend class FrameCallbackInfo;
	friend class NotificationCallbackInfo;
};

Nan::Persistent<v8::Function> RSSensor::constructor_;

void RSSensor::RegisterNotificationCallbackMethod() {
	CallNativeFunc(rs2_set_notifications_callback_cpp, &error_, sensor_, new NotificationCallback(this), &error_);
}

#endif
