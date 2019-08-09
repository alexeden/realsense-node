#ifndef DEVICES_CHANGED_CALLBACK_H
#define DEVICES_CHANGED_CALLBACK_H

#include "context.cc"
#include "main_thread_callback.cc"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

using namespace Napi;

class DevicesChangedCallbackInfo : public MainThreadCallbackInfo {
  public:
	DevicesChangedCallbackInfo(rs2_device_list* r, rs2_device_list* a, RSContext* ctx, Napi::Env env)
	  : removed_(r)
	  , added_(a)
	  , ctx_(ctx)
	  , env_(env) {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::DevicesChangedCallbackInfo"
				  << std::endl;
	}

	~DevicesChangedCallbackInfo() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::~DevicesChangedCallbackInfo"
				  << std::endl;
		if (!consumed_) Release();
	}
	void Run() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::Run" << std::endl;
		SetConsumed();
		std::cerr << __FILE__ << ":" << __LINE__ << " Run" << std::endl;
		auto env = this->env_;
		// env.
		std::cerr << __FILE__ << ":" << __LINE__ << env << std::endl;
		// EscapableHandleScope scope(this->env_);
		// std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
		// // auto handle = scope.Escape(napi_value(this->ctx_)).ToObject();
		// std::cerr << __FILE__ << ":" << __LINE__ << std::endl;

		// // AsyncContext async_ctx(this->env_, "devices_changed_callback_info", scope);
		// std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
		// auto ctx = scope.Escape(napi_value(instance)).ToObject();
		// auto ctx = ObjectWrap<RSContext>::Unwrap(ctx_);
		// HandleScope scope;
		// Value rmList;
		// if (removed_)
		// 	rmList = RSDeviceList::NewInstance(env, removed_);
		// else
		// 	rmList = env.Undefined();

		// Value addList;
		// if (added_)
		// 	addList = RSDeviceList::NewInstance(env, added_);
		// else
		// 	addList = env.Undefined();

		// this->ctx_->Value().Get(this->ctx_->device_changed_callback_name_)
		// this->ctx_->device_changed_callback_.MakeCallback(Napi::Object::New(this->ctx_->Env()), {});
		// this->ctx_->device_changed_callback_.Call({});
		// this->ctx_->device_changed_callback_.MakeCallback(Napi::Object::New(this->env_), {}, async_ctx);
	}
	void Release() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallbackInfo::Release" << std::endl;

		if (removed_) {
			rs2_delete_device_list(removed_);
			removed_ = nullptr;
		}

		if (added_) {
			rs2_delete_device_list(added_);
			added_ = nullptr;
		}
	}

  private:
	rs2_device_list* removed_;
	rs2_device_list* added_;
	// AsyncContext async_ctx_;
	RSContext* ctx_;
	Napi::Env env_;
};

class DevicesChangedCallback : public rs2_devices_changed_callback {
  public:
	DevicesChangedCallback(RSContext* context, Env env, std::shared_ptr<ThreadSafeFunction> fn)
	  : ctx_(context)
	  , env_(env)
	  , fn_(fn) {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallback::DevicesChangedCallback" << std::endl;

		// std::thread nativeThread([fn] {
		// 	// Transform native data into JS data
		// 	auto callback = [](Napi::Env env, Function jsCallback) {
		// 		jsCallback.Call({ });
		// 		// We're finished with it.
		// 		// delete value;
		// 	};
		// 	std::this_thread::sleep_for(std::chrono::seconds(1));

		// 	napi_status status = fn->BlockingCall(callback);
		// 	if (status != napi_status::napi_ok) {
		// 		// Handle error
		// 	}
		// 	// Release the thread-safe function
		// 	fn->Release();
		// });

		// nativeThread.detach();
	}
	virtual void on_devices_changed(rs2_device_list* removed, rs2_device_list* added) {
		std::cerr << __FILE__ << ":" << __LINE__ << "DevicesChangedCallback::on_devices_changed" << this->fn_
				  << std::endl;
		std::cerr << "thread id: " << std::this_thread::get_id() << std::endl;
		// EscapableHandleScope scope(this->env_);
		// auto result = this->ctx_->device_changed_callback_.Call(this, {});
		// scope.Escape(this->ctx_->device_changed_callback_.Call({}));
		// auto callback = [](Napi::Env env, Function jsCallback) {
		// 	jsCallback.Call({ });
		// 	// We're finished with it.
		// 	// delete value;
		// };

		// this->fn_->BlockingCall(callback);
		auto fn = this->fn_;

		if (fn == nullptr) { std::cerr << "FUNCTION IS A NULL POINTER" << std::endl; }
		std::thread nativeThread([fn] {
			std::cerr << "In the thread" << std::endl;
			std::cerr << "thread id: " << std::this_thread::get_id() << std::endl;
			// Transform native data into JS data
			napi_status status = fn->NonBlockingCall([](Napi::Env env, Function jsCallback) {
				std::cerr << "In the call lambda" << std::endl;
				jsCallback.Call({});
				// We're finished with it.
				// delete value;
			});
			std::cerr << "blocking call made" << std::endl;
			if (status != napi_status::napi_ok) {
				std::cerr << "not okay" << std::endl;
				// Handle error
			}
			// Release the thread-safe function
			// fn->Release();
		});

		nativeThread.detach();

		// MainThreadCallback::NotifyMainThread(new DevicesChangedCallbackInfo(removed, added, ctx_, env_));
	}

	virtual void release() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallback::Release" << std::endl;
		// delete this;
		// this->fn_->Release();
	}

	virtual ~DevicesChangedCallback() {
		std::cerr << __FILE__ << ":" << __LINE__ << " DevicesChangedCallback::~DevicesChangedCallback" << std::endl;
	}
	std::shared_ptr<ThreadSafeFunction> fn_;
	RSContext* ctx_;
	Env env_;
};

void RSContext::RegisterDevicesChangedCallbackMethod(Napi::Env env, std::shared_ptr<ThreadSafeFunction> fn) {
	std::cerr << "RSContext::RegisterDevicesChangedCallbackMethod" << std::endl;
	std::cerr << "thread id: " << std::this_thread::get_id() << std::endl;
	rs2_set_devices_changed_callback_cpp(this->ctx_, new DevicesChangedCallback(this, env, fn), &this->error_);
}
// 	// CallNativeFunc(rs2_set_devices_changed_callback_cpp, &error_, this->ctx_, new DevicesChangedCallback(this, env),
// 	// &error_);
// }

// static Value doSomethingUsefulWithData(Env env, void* data) {
// 	// Convert `data` into a JavaScript value and return it.
// }

// // Runs on the JS thread.
// static void FinalizeTSFN(napi_env env, void* data, void* context) {
// 	// This is where you would wait for the threads to quit. This
// 	// function will only be called when all the threads are done using
// 	// the tsfn so, presumably, they can be joined here.
// }

// // Runs on the JS thread.
// static void CallIntoJS(napi_env env, napi_value js_cb, void* context, void* data) {
// 	if (env != nullptr && js_cb != nullptr) {
// 		// `data` was passed to `napi_call_threadsafe_function()` by one
// 		// of the threads. The order in which the threads add data as
// 		// they call `napi_call_threadsafe_function()` is the order in
// 		// which they will be given to this callback.
// 		//
// 		// `context` was passed to `napi_create_threadsafe_function()` and
// 		// is being provided here.
// 		//
// 		Function::New(env, js_cb).Call({ DoSomethingUsefulWithData(env, data) });
// 	}
// 	else {
// 		// The tsfn is in the process of getting cleaned up and there are
// 		// still items left on the queue. This function gets called with
// 		// each `void*` item, but with `env` and `js_cb` set to `NULL`,
// 		// because calls can no longer be made into JS, but the `void*`s
// 		// may still need to be freed.
// 	}
// }

// // Runs on the JS thread.
// static void CreateThreadsafeCallback(const CallbackInfo& info) {
// 	if (!info[0].IsFunction()) {
// 		Error::New(info.Env(), "First argument must be a function").ThrowAsJavaScriptException();
// 		return;
// 	}

// 	napi_threadsafe_function tsfn;

// 	NAPI_THROW_IF_FAILED_VOID(
// 	  info.Env(),
// 	  napi_create_threadsafe_function(
// 		info.Env(),
// 		info[0],
// 		nullptr,
// 		String::New(info.Env(), "My thread-safe function"),
// 		0,			  // for an unlimited queue size
// 		1,			  // initially only used from the main thread
// 		nullptr,	  // data to make use of during finalization
// 		FinalizeTSFN, // gets called when the tsfn goes out of use
// 		nullptr,	  // data that can be set here and retrieved on any thread
// 		CallIntoJS,   // function to call into JS
// 		&tsfn));

// 	// Now you can pass `tsfn` to any number of threads. Each one must
// 	// first call `napi_threadsafe_function_acquire()`. Then it may call
// 	// `napi_call_threadsafe_function()` any number of times. If on one
// 	// of those occasions the return value from
// 	// `napi_call_threadsafe_function()` is `napi_closing`, the thread
// 	// must make no more calls to any of the thread-safe function APIs.
// 	// If it never receives `napi_closing` from
// 	// `napi_call_threadsafe_function()` then, before exiting, the
// 	// thread must call `napi_release_threadsafe_function()`.
// }
#endif
