#ifndef FRAMESET_H
#define FRAMESET_H

class RSFrameSet : public Nan::ObjectWrap {
  public:
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSFrameSet").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "getSize", GetSize);
		Nan::SetPrototypeMethod(tpl, "getFrame", GetFrame);
		Nan::SetPrototypeMethod(tpl, "replaceFrame", ReplaceFrame);
		Nan::SetPrototypeMethod(tpl, "indexToStream", IndexToStream);
		Nan::SetPrototypeMethod(tpl, "indexToStreamIndex", IndexToStreamIndex);

		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSFrameSet").ToLocalChecked(), tpl->GetFunction());
	}

	static v8::Local<v8::Object> NewInstance(rs2_frame* frame) {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor_);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
		auto me						   = Nan::ObjectWrap::Unwrap<RSFrameSet>(instance);
		me->SetFrame(frame);

		return scope.Escape(instance);
	}

	rs2_frame* GetFrames() {
		return frames_;
	}

	void Replace(rs2_frame* frame) {
		DestroyMe();
		SetFrame(frame);
	}

  private:
	RSFrameSet() {
		error_  = nullptr;
		frames_ = nullptr;
	}

	~RSFrameSet() {
		DestroyMe();
	}

	void SetFrame(rs2_frame* frame) {
		if (
		  !frame
		  || (!GetNativeResult<
			  int>(rs2_is_frame_extendable_to, &error_, frame, RS2_EXTENSION_COMPOSITE_FRAME, &error_)))
			return;

		frames_		 = frame;
		frame_count_ = GetNativeResult<int>(rs2_embedded_frames_count, &error_, frame, &error_);
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (frames_) rs2_release_frame(frames_);
		frames_ = nullptr;
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSFrameSet>(info.Holder());
		if (me) { me->DestroyMe(); }
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (info.IsConstructCall()) {
			RSFrameSet* obj = new RSFrameSet();
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	static NAN_METHOD(GetSize) {
		auto me = Nan::ObjectWrap::Unwrap<RSFrameSet>(info.Holder());
		if (me && me->frames_) {
			info.GetReturnValue().Set(Nan::New(me->frame_count_));
			return;
		}
		info.GetReturnValue().Set(Nan::New(0));
	}

	static NAN_METHOD(GetFrame) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSFrameSet>(info.Holder());
		if (!me || !me->frames_) return;

		rs2_stream stream = static_cast<rs2_stream>(info[0]->IntegerValue());
		auto stream_index = info[1]->IntegerValue();
		// if RS2_STREAM_ANY is used, we return the first frame.
		if (stream == RS2_STREAM_ANY && me->frame_count_) {
			rs2_frame* frame = GetNativeResult<rs2_frame*>(rs2_extract_frame, &me->error_, me->frames_, 0, &me->error_);
			if (!frame) return;

			info.GetReturnValue().Set(RSFrame::NewInstance(frame));
			return;
		}

		for (uint32_t i = 0; i < me->frame_count_; i++) {
			rs2_frame* frame = GetNativeResult<rs2_frame*>(rs2_extract_frame, &me->error_, me->frames_, i, &me->error_);
			if (!frame) continue;

			const rs2_stream_profile* profile = GetNativeResult<
			  const rs2_stream_profile*>(rs2_get_frame_stream_profile, &me->error_, frame, &me->error_);
			if (profile) {
				StreamProfileExtrator extrator(profile);
				if (
				  extrator.stream_ == stream && (!stream_index || (stream_index && stream_index == extrator.index_))) {
					info.GetReturnValue().Set(RSFrame::NewInstance(frame));
					return;
				}
			}
			rs2_release_frame(frame);
		}
	}

	static NAN_METHOD(ReplaceFrame) {
		info.GetReturnValue().Set(Nan::False());
		auto me			  = Nan::ObjectWrap::Unwrap<RSFrameSet>(info.Holder());
		rs2_stream stream = static_cast<rs2_stream>(info[0]->IntegerValue());
		auto stream_index = info[1]->IntegerValue();
		auto target_frame = Nan::ObjectWrap::Unwrap<RSFrame>(info[2]->ToObject());

		if (!me || !me->frames_) return;

		for (uint32_t i = 0; i < me->frame_count_; i++) {
			rs2_frame* frame = GetNativeResult<rs2_frame*>(rs2_extract_frame, &me->error_, me->frames_, i, &me->error_);
			if (!frame) continue;

			const rs2_stream_profile* profile = GetNativeResult<
			  const rs2_stream_profile*>(rs2_get_frame_stream_profile, &me->error_, frame, &me->error_);
			if (profile) {
				StreamProfileExtrator extrator(profile);
				if (
				  extrator.stream_ == stream && (!stream_index || (stream_index && stream_index == extrator.index_))) {
					target_frame->Replace(frame);
					info.GetReturnValue().Set(Nan::True());
					return;
				}
			}
			rs2_release_frame(frame);
		}
	}

	static NAN_METHOD(IndexToStream) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSFrameSet>(info.Holder());
		if (!me || !me->frames_) return;

		int32_t index	= info[0]->IntegerValue();
		rs2_frame* frame = GetNativeResult<rs2_frame*>(rs2_extract_frame, &me->error_, me->frames_, index, &me->error_);
		if (!frame) return;

		const rs2_stream_profile* profile
		  = GetNativeResult<const rs2_stream_profile*>(rs2_get_frame_stream_profile, &me->error_, frame, &me->error_);
		if (!profile) {
			rs2_release_frame(frame);
			return;
		}

		rs2_stream stream = RS2_STREAM_ANY;
		rs2_format format = RS2_FORMAT_ANY;
		int32_t fps		  = 0;
		int32_t idx		  = 0;
		int32_t unique_id = 0;
		CallNativeFunc(rs2_get_stream_profile_data, &me->error_, profile, &stream, &format, &idx, &unique_id, &fps, &me->error_);
		rs2_release_frame(frame);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(stream));
	}

	static NAN_METHOD(IndexToStreamIndex) {
		auto me = Nan::ObjectWrap::Unwrap<RSFrameSet>(info.Holder());
		info.GetReturnValue().Set(Nan::Undefined());
		if (!me || !me->frames_) return;

		int32_t index	= info[0]->IntegerValue();
		rs2_frame* frame = GetNativeResult<rs2_frame*>(rs2_extract_frame, &me->error_, me->frames_, index, &me->error_);
		if (!frame) return;

		const rs2_stream_profile* profile
		  = GetNativeResult<const rs2_stream_profile*>(rs2_get_frame_stream_profile, &me->error_, frame, &me->error_);
		if (!profile) {
			rs2_release_frame(frame);
			return;
		}
		rs2_stream stream = RS2_STREAM_ANY;
		rs2_format format = RS2_FORMAT_ANY;
		int32_t fps		  = 0;
		int32_t idx		  = 0;
		int32_t unique_id = 0;
		CallNativeFunc(rs2_get_stream_profile_data, &me->error_, profile, &stream, &format, &idx, &unique_id, &fps, &me->error_);
		rs2_release_frame(frame);
		if (me->error_) return;

		info.GetReturnValue().Set(Nan::New(idx));
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;
	rs2_frame* frames_;
	uint32_t frame_count_;
	rs2_error* error_;
};

Nan::Persistent<v8::Function> RSFrameSet::constructor_;

#endif
