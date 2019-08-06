#ifndef ERRORUTIL_H
#define ERRORUTIL_H

#include <iostream>
#include <string>
#include <napi.h>
#include <librealsense2/hpp/rs_types.hpp>
#include "dict_base.cc"

using namespace Napi;

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
		Object GetJSObject(Env env) {
			auto obj = Object::New(env);
			obj.Set("recoverable", recoverable_);
			obj.Set("description", description_);
			obj.Set("nativeFunction", native_function_);

			return obj;
		}

	  private:
		bool is_error_;
		bool recoverable_;
		std::string description_;
		std::string native_function_;

		friend class ErrorUtil;
	};

	ErrorUtil(Env env)
	  : env_(env) {
	}

	~ErrorUtil() {
	}

	static void Init(Env env) {
		if (!singleton_) singleton_ = new ErrorUtil(env);
	}

	/**
	 * info[0] -> The object with a key whose value is the function to call
	 * info[1] -> The key of the function to call
	 */
	static void UpdateJSErrorCallback(const CallbackInfo& info) {
		singleton_->js_error_container_.Reset(info[0].As<Object>());
		auto value = std::string(info[1].As<String>());
		singleton_->js_error_callback_name_ = value;
	}

	static void AnalyzeError(rs2_error* err) {
		if (!err) return;

		auto function	= std::string(rs2_get_failed_function(err));
		auto type		 = rs2_get_librealsense_exception_type(err);
		auto msg		 = std::string(rs2_get_error_message(err));
		bool recoverable = false;

		if (
		  type == RS2_EXCEPTION_TYPE_INVALID_VALUE || type == RS2_EXCEPTION_TYPE_WRONG_API_CALL_SEQUENCE
		  || type == RS2_EXCEPTION_TYPE_NOT_IMPLEMENTED) {
			recoverable = true;
		}

		singleton_->MarkError(recoverable, msg, function);
	}

	static void ResetError() {
		singleton_->error_info_.Reset();
	}

	static Value GetJSErrorObject(Env env) {
		if (singleton_->error_info_.is_error_) return singleton_->error_info_.GetJSObject(env);

		return env.Undefined();
	}

  private:
	// Save detailed error info to the js object
	void MarkError(bool recoverable, std::string description, std::string native_function) {
		error_info_.Update(true, recoverable, description, native_function);
		auto cb = js_error_container_.Get(js_error_callback_name_.c_str()).As<Function>();
		cb.Call({ GetJSErrorObject(env_) });
	}

	static ErrorUtil* singleton_;
	Env env_;
	ErrorInfo error_info_;
	ObjectReference js_error_container_;
	std::string js_error_callback_name_;
};

ErrorUtil* ErrorUtil::singleton_ = nullptr;

#endif
