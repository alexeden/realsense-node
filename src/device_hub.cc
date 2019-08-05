#ifndef DEVICE_HUB_H
#define DEVICE_HUB_H

#include <napi.h>

class RSDeviceHub : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RSDeviceHub").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "waitForDevice", WaitForDevice);
    Nan::SetPrototypeMethod(tpl, "isConnected", IsConnected);
    Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
    constructor_.Reset(tpl->GetFunction());
    exports->Set(Nan::New("RSDeviceHub").ToLocalChecked(),
      tpl->GetFunction());
  }

  static v8::Local<v8::Object> NewInstance() {
    Nan::EscapableHandleScope scope;

    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor_);
    v8::Local<v8::Context> context =
        v8::Isolate::GetCurrent()->GetCurrentContext();

    v8::Local<v8::Object> instance =
        cons->NewInstance(context, 0, nullptr).ToLocalChecked();

    return scope.Escape(instance);
  }

 private:
  RSDeviceHub() : hub_(nullptr), ctx_(nullptr), error_(nullptr) {}

  ~RSDeviceHub() {
    DestroyMe();
  }

  void DestroyMe() {
    if (error_) rs2_free_error(error_);
    error_ = nullptr;

    if (hub_) rs2_delete_device_hub(hub_);
    hub_ = nullptr;
    ctx_ = nullptr;
  }

  static NAN_METHOD(Destroy) {
    auto me = Nan::ObjectWrap::Unwrap<RSDeviceHub>(info.Holder());
    if (me) me->DestroyMe();
    info.GetReturnValue().Set(Nan::Undefined());
  }

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.IsConstructCall()) {
      RSDeviceHub* obj = new RSDeviceHub();
      RSContext* ctx = Nan::ObjectWrap::Unwrap<RSContext>(info[0]->ToObject());
      obj->ctx_ = ctx->ctx_;
      obj->hub_ = GetNativeResult<rs2_device_hub*>(rs2_create_device_hub,
          &obj->error_, obj->ctx_, &obj->error_);
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }

  static NAN_METHOD(WaitForDevice) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSDeviceHub>(info.Holder());
    if (!me) return;

    auto dev = GetNativeResult<rs2_device*>(rs2_device_hub_wait_for_device,
        &me->error_, me->hub_, &me->error_);
    if (!dev) return;

    info.GetReturnValue().Set(RSDevice::NewInstance(dev));
  }

  static NAN_METHOD(IsConnected) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSDeviceHub>(info.Holder());
    auto dev = Nan::ObjectWrap::Unwrap<RSDevice>(info[0]->ToObject());
    if (!me || !dev) return;

    auto res = GetNativeResult<int>(rs2_device_hub_is_device_connected,
       &me->error_, me->hub_, dev->dev_, &me->error_);
    if (me->error_) return;

    info.GetReturnValue().Set(Nan::New(res ? true : false));
  }

 private:
  static Nan::Persistent<v8::Function> constructor_;

  rs2_device_hub* hub_;
  rs2_context* ctx_;
  rs2_error* error_;
};

Nan::Persistent<v8::Function> RSDeviceHub::constructor_;

#endif
