#ifndef COLORIZER_H
#define COLORIZER_H

#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSColorizer
  : public Nan::ObjectWrap
  , Options {
  public:
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSColorizer").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "create", Create);
		Nan::SetPrototypeMethod(tpl, "colorize", Colorize);
		Nan::SetPrototypeMethod(tpl, "supportsOption", SupportsOption);
		Nan::SetPrototypeMethod(tpl, "getOption", GetOption);
		Nan::SetPrototypeMethod(tpl, "setOption", SetOption);
		Nan::SetPrototypeMethod(tpl, "getOptionRange", GetOptionRange);
		Nan::SetPrototypeMethod(tpl, "isOptionReadonly", IsOptionReadonly);
		Nan::SetPrototypeMethod(tpl, "getOptionDescription", GetOptionDescription);
		Nan::SetPrototypeMethod(tpl, "getOptionValueDescription", GetOptionValueDescription);

		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSColorizer").ToLocalChecked(), tpl->GetFunction());
	}

	static v8::Local<v8::Object> NewInstance() {
		Nan::EscapableHandleScope scope;

		v8::Local<v8::Function> cons   = Nan::New<v8::Function>(constructor_);
		v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();

		v8::Local<v8::Object> instance = cons->NewInstance(context, 0, nullptr).ToLocalChecked();

		return scope.Escape(instance);
	}

	rs2_options* GetOptionsPointer() override {
		// TODO(shaoting) find better way to avoid the reinterpret_cast which was
		// caused the inheritance relation was hidden
		return reinterpret_cast<rs2_options*>(colorizer_);
	}

  private:
	RSColorizer()
	  : colorizer_(nullptr)
	  , frame_queue_(nullptr)
	  , error_(nullptr) {
	}

	~RSColorizer() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (colorizer_) rs2_delete_processing_block(colorizer_);
		colorizer_ = nullptr;
		if (frame_queue_) rs2_delete_frame_queue(frame_queue_);
		frame_queue_ = nullptr;
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) { me->DestroyMe(); }
		info.GetReturnValue().Set(Nan::Undefined());
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (info.IsConstructCall()) {
			RSColorizer* obj = new RSColorizer();
			obj->Wrap(info.This());
			info.GetReturnValue().Set(info.This());
		}
	}

	static NAN_METHOD(Create) {
		info.GetReturnValue().Set(Nan::Undefined());
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (!me) return;

		me->colorizer_ = GetNativeResult<rs2_processing_block*>(rs2_create_colorizer, &me->error_, &me->error_);
		if (!me->colorizer_) return;

		me->frame_queue_ = GetNativeResult<rs2_frame_queue*>(rs2_create_frame_queue, &me->error_, 1, &me->error_);
		if (!me->frame_queue_) return;

		auto callback = new FrameCallbackForFrameQueue(me->frame_queue_);
		CallNativeFunc(rs2_start_processing, &me->error_, me->colorizer_, callback, &me->error_);
	}

	static NAN_METHOD(Colorize) {
		info.GetReturnValue().Set(Nan::False());
		auto me			= Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		RSFrame* depth  = Nan::ObjectWrap::Unwrap<RSFrame>(info[0]->ToObject());
		RSFrame* target = Nan::ObjectWrap::Unwrap<RSFrame>(info[1]->ToObject());
		if (!me || !depth || !depth->frame_ || !target) return;

		// rs2_process_frame will release the input frame, so we need to addref
		CallNativeFunc(rs2_frame_add_ref, &me->error_, depth->frame_, &me->error_);
		if (me->error_) return;

		CallNativeFunc(rs2_process_frame, &me->error_, me->colorizer_, depth->frame_, &me->error_);
		if (me->error_) return;

		rs2_frame* result
		  = GetNativeResult<rs2_frame*>(rs2_wait_for_frame, &me->error_, me->frame_queue_, 5000, &me->error_);
		target->DestroyMe();
		if (!result) return;

		target->Replace(result);
		info.GetReturnValue().Set(Nan::True());
	}

	static NAN_METHOD(SupportsOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) return me->SupportsOptionInternal(info);

		info.GetReturnValue().Set(Nan::False());
	}

	static NAN_METHOD(GetOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) return me->GetOptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionDescription) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) return me->GetOptionDescriptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionValueDescription) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) return me->GetOptionValueDescriptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(SetOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) return me->SetOptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionRange) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) return me->GetOptionRangeInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(IsOptionReadonly) {
		auto me = Nan::ObjectWrap::Unwrap<RSColorizer>(info.Holder());
		if (me) return me->IsOptionReadonlyInternal(info);

		info.GetReturnValue().Set(Nan::False());
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;

	rs2_processing_block* colorizer_;
	rs2_frame_queue* frame_queue_;
	rs2_error* error_;
};

Nan::Persistent<v8::Function> RSColorizer::constructor_;

#endif
