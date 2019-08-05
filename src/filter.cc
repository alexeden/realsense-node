#ifndef COLORIZER_H
#define COLORIZER_H

#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSFilter
  : public Nan::ObjectWrap
  , Options {
  public:
	enum FilterType {
		kFilterDecimation = 0,
		kFilterTemporal,
		kFilterSpatial,
		kFilterHoleFilling,
		kFilterDisparity2Depth,
		kFilterDepth2Disparity
	};
	static void Init(v8::Local<v8::Object> exports) {
		v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New("RSFilter").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
		Nan::SetPrototypeMethod(tpl, "process", Process);
		Nan::SetPrototypeMethod(tpl, "supportsOption", SupportsOption);
		Nan::SetPrototypeMethod(tpl, "getOption", GetOption);
		Nan::SetPrototypeMethod(tpl, "setOption", SetOption);
		Nan::SetPrototypeMethod(tpl, "getOptionRange", GetOptionRange);
		Nan::SetPrototypeMethod(tpl, "isOptionReadonly", IsOptionReadonly);
		Nan::SetPrototypeMethod(tpl, "getOptionDescription", GetOptionDescription);
		Nan::SetPrototypeMethod(tpl, "getOptionValueDescription", GetOptionValueDescription);

		constructor_.Reset(tpl->GetFunction());
		exports->Set(Nan::New("RSFilter").ToLocalChecked(), tpl->GetFunction());
	}

	rs2_options* GetOptionsPointer() override {
		return reinterpret_cast<rs2_options*>(block_);
	}

  private:
	RSFilter()
	  : block_(nullptr)
	  , frame_queue_(nullptr)
	  , error_(nullptr)
	  , type_(kFilterDecimation) {
	}

	~RSFilter() {
		DestroyMe();
	}

	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (block_) rs2_delete_processing_block(block_);
		block_ = nullptr;
		if (frame_queue_) rs2_delete_frame_queue(frame_queue_);
		frame_queue_ = nullptr;
	}

	static NAN_METHOD(Destroy) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) me->DestroyMe();

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		if (!info.IsConstructCall()) return;

		v8::String::Utf8Value type_str(info[0]);
		std::string type = std::string(*type_str);
		RSFilter* obj	= new RSFilter();
		if (!(type.compare("decimation"))) {
			obj->type_ = kFilterDecimation;
			obj->block_
			  = GetNativeResult<rs2_processing_block*>(rs2_create_decimation_filter_block, &obj->error_, &obj->error_);
		}
		else if (!(type.compare("temporal"))) {
			obj->type_ = kFilterTemporal;
			obj->block_
			  = GetNativeResult<rs2_processing_block*>(rs2_create_temporal_filter_block, &obj->error_, &obj->error_);
		}
		else if (!(type.compare("spatial"))) {
			obj->type_ = kFilterSpatial;
			obj->block_
			  = GetNativeResult<rs2_processing_block*>(rs2_create_spatial_filter_block, &obj->error_, &obj->error_);
		}
		else if (!(type.compare("hole-filling"))) {
			obj->type_  = kFilterHoleFilling;
			obj->block_ = GetNativeResult<
			  rs2_processing_block*>(rs2_create_hole_filling_filter_block, &obj->error_, &obj->error_);
		}
		else if (!(type.compare("disparity-to-depth"))) {
			obj->type_  = kFilterDisparity2Depth;
			obj->block_ = GetNativeResult<
			  rs2_processing_block*>(rs2_create_disparity_transform_block, &obj->error_, 0, &obj->error_);
		}
		else if (!(type.compare("depth-to-disparity"))) {
			obj->type_  = kFilterDepth2Disparity;
			obj->block_ = GetNativeResult<
			  rs2_processing_block*>(rs2_create_disparity_transform_block, &obj->error_, 1, &obj->error_);
		}
		if (!obj->block_) return;

		obj->frame_queue_ = GetNativeResult<rs2_frame_queue*>(rs2_create_frame_queue, &obj->error_, 1, &obj->error_);
		if (!obj->frame_queue_) return;

		auto callback = new FrameCallbackForFrameQueue(obj->frame_queue_);
		CallNativeFunc(rs2_start_processing, &obj->error_, obj->block_, callback, &obj->error_);
		if (obj->error_) return;

		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}

	static NAN_METHOD(Process) {
		auto me			 = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		auto input_frame = Nan::ObjectWrap::Unwrap<RSFrame>(info[0]->ToObject());
		auto out_frame   = Nan::ObjectWrap::Unwrap<RSFrame>(info[1]->ToObject());
		info.GetReturnValue().Set(Nan::False());
		if (!me || !input_frame || !out_frame) return;

		// rs2_process_frame will release the input frame, so we need to addref
		CallNativeFunc(rs2_frame_add_ref, &me->error_, input_frame->frame_, &me->error_);
		if (me->error_) return;

		CallNativeFunc(rs2_process_frame, &me->error_, me->block_, input_frame->frame_, &me->error_);
		if (me->error_) return;

		rs2_frame* frame = nullptr;
		auto ret_code	= GetNativeResult<int>(rs2_poll_for_frame, &me->error_, me->frame_queue_, &frame, &me->error_);
		if (!ret_code) return;

		out_frame->Replace(frame);
		info.GetReturnValue().Set(Nan::True());
	}

	static NAN_METHOD(SupportsOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) return me->SupportsOptionInternal(info);

		info.GetReturnValue().Set(Nan::False());
	}

	static NAN_METHOD(GetOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) return me->GetOptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionDescription) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) return me->GetOptionDescriptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionValueDescription) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) return me->GetOptionValueDescriptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(SetOption) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) return me->SetOptionInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(GetOptionRange) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) return me->GetOptionRangeInternal(info);

		info.GetReturnValue().Set(Nan::Undefined());
	}

	static NAN_METHOD(IsOptionReadonly) {
		auto me = Nan::ObjectWrap::Unwrap<RSFilter>(info.Holder());
		if (me) return me->IsOptionReadonlyInternal(info);

		info.GetReturnValue().Set(Nan::False());
	}

  private:
	static Nan::Persistent<v8::Function> constructor_;

	rs2_processing_block* block_;
	rs2_frame_queue* frame_queue_;
	rs2_error* error_;
	FilterType type_;
};

Nan::Persistent<v8::Function> RSFilter::constructor_;

#endif
