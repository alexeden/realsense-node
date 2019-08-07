#ifndef COLORIZER_H
#define COLORIZER_H

#include "frame.cc"
#include "frame_callbacks.cc"
#include "options.cc"
#include "utils.cc"
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class RSColorizer
  : public ObjectWrap<RSColorizer>
  , Options {
  public:
	static Object Init(Napi::Env env, Object exports) {
		Napi::Function func = DefineClass(
		  env,
		  "RSColorizer",
		  {
			InstanceMethod("destroy", &RSColorizer::Destroy),
			InstanceMethod("create", &RSColorizer::Create),
			InstanceMethod("colorize", &RSColorizer::Colorize),
			InstanceMethod("supportsOption", &RSColorizer::SupportsOption),
			InstanceMethod("getOption", &RSColorizer::GetOption),
			InstanceMethod("setOption", &RSColorizer::SetOption),
			InstanceMethod("getOptionRange", &RSColorizer::GetOptionRange),
			InstanceMethod("isOptionReadonly", &RSColorizer::IsOptionReadonly),
			InstanceMethod("getOptionDescription", &RSColorizer::GetOptionDescription),
			InstanceMethod("getOptionValueDescription", &RSColorizer::GetOptionValueDescription),

		  });

		constructor = Napi::Persistent(func);
		constructor.SuppressDestruct();
		exports.Set("RSColorizer", func);

		return exports;
	}

	static Object NewInstance(Napi::Env env) {
		EscapableHandleScope scope(env);
		Object instance = constructor.New({});

		return scope.Escape(napi_value(instance)).ToObject();
	}

	rs2_options* GetOptionsPointer() override {
		// TODO(shaoting) find better way to avoid the reinterpret_cast which was
		// caused the inheritance relation was hidden
		return reinterpret_cast<rs2_options*>(colorizer_);
	}
	RSColorizer(const CallbackInfo& info)
	  : ObjectWrap<RSColorizer>(info)
	  , colorizer_(nullptr)
	  , frame_queue_(nullptr)
	  , error_(nullptr) {
	}

	~RSColorizer() {
		DestroyMe();
	}

  private:
	void DestroyMe() {
		if (error_) rs2_free_error(error_);
		error_ = nullptr;
		if (colorizer_) rs2_delete_processing_block(colorizer_);
		colorizer_ = nullptr;
		if (frame_queue_) rs2_delete_frame_queue(frame_queue_);
		frame_queue_ = nullptr;
	}

	Napi::Value Destroy(const CallbackInfo& info) {
		this->DestroyMe();
		return info.Env().Undefined();
	}

	Napi::Value Create(const CallbackInfo& info) {
		this->colorizer_ = GetNativeResult<rs2_processing_block*>(rs2_create_colorizer, &this->error_, &this->error_);
		if (!this->colorizer_) return info.Env().Undefined();

		this->frame_queue_ = GetNativeResult<rs2_frame_queue*>(rs2_create_frame_queue, &this->error_, 1, &this->error_);
		if (!this->frame_queue_) return info.Env().Undefined();

		auto callback = new FrameCallbackForFrameQueue(this->frame_queue_);
		CallNativeFunc(rs2_start_processing, &this->error_, this->colorizer_, callback, &this->error_);
		return info.Env().Undefined();
	}

	Napi::Value Colorize(const CallbackInfo& info) {
		RSFrame* depth  = ObjectWrap<RSFrame>::Unwrap(info[0].ToObject());
		RSFrame* target = ObjectWrap<RSFrame>::Unwrap(info[1].ToObject());
		if (!depth || !depth->frame_ || !target) return Boolean::New(info.Env(), false);

		// rs2_process_frame will release the input frame, so we need to addref
		CallNativeFunc(rs2_frame_add_ref, &this->error_, depth->frame_, &this->error_);
		if (this->error_) return Boolean::New(info.Env(), false);

		CallNativeFunc(rs2_process_frame, &this->error_, this->colorizer_, depth->frame_, &this->error_);
		if (this->error_) return Boolean::New(info.Env(), false);

		rs2_frame* result
		  = GetNativeResult<rs2_frame*>(rs2_wait_for_frame, &this->error_, this->frame_queue_, 5000, &this->error_);
		target->DestroyMe();
		if (!result) return Boolean::New(info.Env(), false);

		target->Replace(result);

		return Boolean::New(info.Env(), true);
	}

	Napi::Value SupportsOption(const CallbackInfo& info) {
		return this->SupportsOptionInternal(info);
	}

	Napi::Value GetOption(const CallbackInfo& info) {
		return this->GetOptionInternal(info);
	}

	Napi::Value GetOptionDescription(const CallbackInfo& info) {
		return this->GetOptionDescriptionInternal(info);
	}

	Napi::Value GetOptionValueDescription(const CallbackInfo& info) {
		return this->GetOptionValueDescriptionInternal(info);
	}

	Napi::Value SetOption(const CallbackInfo& info) {
		return this->SetOptionInternal(info);
	}

	Napi::Value GetOptionRange(const CallbackInfo& info) {
		return this->GetOptionRangeInternal(info);
	}

	Napi::Value IsOptionReadonly(const CallbackInfo& info) {
		return this->IsOptionReadonlyInternal(info);
	}

  private:
	static FunctionReference constructor;

	rs2_processing_block* colorizer_;
	rs2_frame_queue* frame_queue_;
	rs2_error* error_;
};

Napi::FunctionReference RSColorizer::constructor;

#endif
