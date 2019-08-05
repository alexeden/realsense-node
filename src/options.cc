#ifndef OPTIONS_H
#define OPTIONS_H

class Options {
  public:
	Options()
	  : error_(nullptr) {
	}

	virtual ~Options() {
		if (error_) rs2_free_error(error_);
	}

	virtual rs2_options* GetOptionsPointer() = 0;

	void SupportsOptionInternal(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		int32_t option = info[0]->IntegerValue();
		auto on		   = GetNativeResult<
			 int>(rs2_supports_option, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);
		info.GetReturnValue().Set(Nan::New(on ? true : false));
		return;
	}

	void GetOptionInternal(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		int32_t option = info[0]->IntegerValue();
		auto value	 = GetNativeResult<
		  float>(rs2_get_option, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);
		info.GetReturnValue().Set(Nan::New(value));
	}

	void GetOptionDescriptionInternal(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		int32_t option = info[0]->IntegerValue();
		auto desc	  = GetNativeResult<
		   const char*>(rs2_get_option_description, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);
		if (desc)
			info.GetReturnValue().Set(Nan::New(desc).ToLocalChecked());
		else
			info.GetReturnValue().Set(Nan::Undefined());
	}

	void GetOptionValueDescriptionInternal(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		int32_t option = info[0]->IntegerValue();
		auto val	   = info[1]->NumberValue();
		auto desc	  = GetNativeResult<
		   const char*>(rs2_get_option_value_description, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), val, &error_);
		if (desc)
			info.GetReturnValue().Set(Nan::New(desc).ToLocalChecked());
		else
			info.GetReturnValue().Set(Nan::Undefined());
	}

	void SetOptionInternal(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		int32_t option = info[0]->IntegerValue();
		auto val	   = info[1]->NumberValue();
		CallNativeFunc(rs2_set_option, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), val, &error_);
		info.GetReturnValue().Set(Nan::Undefined());
	}

	void GetOptionRangeInternal(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		int32_t option = info[0]->IntegerValue();
		float min	  = 0;
		float max	  = 0;
		float step	 = 0;
		float def	  = 0;
		CallNativeFunc(
		  rs2_get_option_range,
		  &error_,
		  GetOptionsPointer(),
		  static_cast<rs2_option>(option),
		  &min,
		  &max,
		  &step,
		  &def,
		  &error_);
		info.GetReturnValue().Set(RSOptionRange(min, max, step, def).GetObject());
	}

	void IsOptionReadonlyInternal(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		int32_t option = info[0]->IntegerValue();
		auto val	   = GetNativeResult<
			int>(rs2_is_option_read_only, &error_, GetOptionsPointer(), static_cast<rs2_option>(option), &error_);
		info.GetReturnValue().Set(Nan::New((val) ? true : false));
	}

  private:
	rs2_error* error_;
};

#endif
