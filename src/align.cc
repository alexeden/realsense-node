#ifndef ALIGN_H
#define ALIGN_H

#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSAlign : public Nan::ObjectWrap {
  public:
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSAlign").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "waitForFrames", WaitForFrames);
		Nan::SetPrototypeMethod(tpl, "process", Process);

		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSAlign").ToLocalChecked(), tpl->GetFunction());
	}

  private:
	RSAlign()
	  : align_(nullptr)
	  , frame_queue_(nullptr)
	  , error_(nullptr) {
	}

	~RSAlign() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (align_) rs2_delete_processing_block(align_);
		align_ = nullptr;
		if (frame_queue_) rs2_delete_frame_queue(frame_queue_);
		frame_queue_ = nullptr;
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSAlign>(info.Holder());
		if (me) me->DestroyMe();

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (!info.IsConstructCall()) return;

		RSAlign* obj = new RSAlign();
		auto stream  = static_cast<rs2_stream>(info[0]->IntegerValue());
		obj->align_  = GetNativeResult<rs2_processing_block*>(rs2_create_align, &obj->error_, stream, &obj->error_);
		if (!obj->align_) return;

		obj->frame_queue_ = GetNativeResult<rs2_frame_queue*>(rs2_create_frame_queue, &obj->error_, 1, &obj->error_);
		if (!obj->frame_queue_) return;

		auto callback = new FrameCallbackForFrameQueue(obj->frame_queue_);
		CallNativeFunc(rs2_start_processing, &obj->error_, obj->align_, callback, &obj->error_);

		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}

	static NAN_METHOD(WaitForFrames) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSAlign>(info.Holder());
		if (!me) return;

		rs2_frame* result
		  = GetNativeResult<rs2_frame*>(rs2_wait_for_frame, &me->error_, me->frame_queue_, 5000, &me->error_);
		if (!result) return;

		info.GetReturnValue().Set(RSFrameSet::NewInstance(result));
	}

	static NAN_METHOD(Process) {
		info.GetReturnValue().Set(Nan::False());
		auto me		   = Nan::ObjectWrap::Unwrap<RSAlign>(info.Holder());
		auto frameset  = Nan::ObjectWrap::Unwrap<RSFrameSet>(info[0]->ToObject());
		auto target_fs = Nan::ObjectWrap::Unwrap<RSFrameSet>(info[1]->ToObject());
		if (!me || !frameset || !target_fs) return;

		// rs2_process_frame will release the input frame, so we need to addref
		CallNativeFunc(rs2_frame_add_ref, &me->error_, frameset->GetFrames(), &me->error_);
		if (me->error_) return;

		CallNativeFunc(rs2_process_frame, &me->error_, me->align_, frameset->GetFrames(), &me->error_);
		if (me->error_) return;

		rs2_frame* frame = nullptr;
		auto ret_code	= GetNativeResult<int>(rs2_poll_for_frame, &me->error_, me->frame_queue_, &frame, &me->error_);
		if (!ret_code) return;

		target_fs->Replace(frame);
		info.GetReturnValue().Set(Nan::True());
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;

	rs2_processing_block* align_;
	rs2_frame_queue* frame_queue_;
	rs2_error* error_;
	friend class RSPipeline;
};

Nan::Persistent<v8::Function> RSAlign::constructor_;

#endif
