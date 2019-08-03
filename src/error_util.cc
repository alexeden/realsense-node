#ifndef ERRORUTIL_H
#define ERRORUTIL_H

#include <string>
#include <napi.h>
#include "dict_base.cc";

class ErrorUtil {
  public:
	class ErrorInfo {
	  public:
		ErrorInfo()
		  : is_error_(false)
		  , recoverable_(false) {
		}

		~ErrorInfo() {
		}

		void Update(bool is_error, bool recoverable, std::string description, std::string function) {
			is_error_		 = is_error;
			recoverable_	 = recoverable;
			description_	 = description;
			native_function_ = function;
		}

		void Reset() {
			Update(false, false, "", "");
		}

		// set value to js attributes only when this method is called
		Napi::Object GetJSObject() {
			DictBase obj;
			obj.SetMemberT("recoverable", recoverable_);
			obj.SetMember("description", description_);
			obj.SetMember("nativeFunction", native_function_);
			return obj.GetObject();
		}

	  private:
		bool is_error_;
		bool recoverable_;
		std::string description_;
		std::string native_function_;

		friend class ErrorUtil;
	};

	~ErrorUtil() {
	}

	static void Init() {
		if (!singleton_) singleton_ = new ErrorUtil();
	}

	static void UpdateJSErrorCallback(const Nan::FunctionCallbackInfo<v8::Value>& info) {
		singleton_->js_error_container_.Reset(info[0]->ToObject());
		v8::String::Utf8Value value(info[1]->ToString());
		singleton_->js_error_callback_name_ = std::string(*value);
	}

	static void AnalyzeError(rs2_error* err) {
		if (!err) return;

		auto function	 = std::string(rs2_get_failed_function(err));
		auto type		 = rs2_get_librealsense_exception_type(err);
		auto msg		 = std::string(rs2_get_error_message(err));
		bool recoverable = false;

		if (
		  type == RS2_EXCEPTION_TYPE_INVALID_VALUE
		  || type == RS2_EXCEPTION_TYPE_WRONG_API_CALL_SEQUENCE
		  || type == RS2_EXCEPTION_TYPE_NOT_IMPLEMENTED
		) {
			recoverable = true;
		}

		singleton_->MarkError(recoverable, msg, function);
	}

	static void ResetError() {
		singleton_->error_info_.Reset();
	}

	static v8::Local<v8::Value> GetJSErrorObject() {
		if (singleton_->error_info_.is_error_) return singleton_->error_info_.GetJSObject();

		return Nan::Undefined();
	}

  private:
	// Save detailed error info to the js object
	void MarkError(bool recoverable, std::string description, std::string native_function) {
		error_info_.Update(true, recoverable, description, native_function);
		v8::Local<v8::Value> args[1] = { GetJSErrorObject() };
		auto container				 = Nan::New<v8::Object>(js_error_container_);
		Nan::MakeCallback(container, js_error_callback_name_.c_str(), 1, args);
	}

	static ErrorUtil* singleton_;
	ErrorInfo error_info_;
	Nan::Persistent<v8::Object> js_error_container_;
	std::string js_error_callback_name_;
};

ErrorUtil* ErrorUtil::singleton_ = nullptr;

template<typename R, typename F, typename... arguments>
R GetNativeResult(F func, rs2_error** error, arguments... params) {
	// reset the error pointer for each call.
	*error = nullptr;
	ErrorUtil::ResetError();
	R val = func(params...);
	ErrorUtil::AnalyzeError(*error);
	return val;
}

template<typename F, typename... arguments>
void CallNativeFunc(F func, rs2_error** error, arguments... params) {
	// reset the error pointer for each call.
	*error = nullptr;
	ErrorUtil::ResetError();
	func(params...);
	ErrorUtil::AnalyzeError(*error);
}

#endif
