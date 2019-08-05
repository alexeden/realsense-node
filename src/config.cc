#ifndef CONFIG_H
#define CONFIG_H

#include <librealsense2/hpp/rs_types.hpp>
#include <napi.h>

class RSConfig : public Nan::ObjectWrap  {
 public:
  static void Init(v8::Local<v8::Object> exports) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RSConfig").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
    Nan::SetPrototypeMethod(tpl, "enableStream", EnableStream);
    Nan::SetPrototypeMethod(tpl, "enableAllStreams", EnableAllStreams);
    Nan::SetPrototypeMethod(tpl, "enableDevice", EnableDevice);
    Nan::SetPrototypeMethod(tpl, "enableDeviceFromFile", EnableDeviceFromFile);
    Nan::SetPrototypeMethod(tpl, "enableRecordToFile", EnableRecordToFile);
    Nan::SetPrototypeMethod(tpl, "disableStream", DisableStream);
    Nan::SetPrototypeMethod(tpl, "disableAllStreams", DisableAllStreams);
    Nan::SetPrototypeMethod(tpl, "resolve", Resolve);
    Nan::SetPrototypeMethod(tpl, "canResolve", CanResolve);
    Nan::SetPrototypeMethod(tpl, "enableDeviceFromFileRepeatOption",
        EnableDeviceFromFileRepeatOption);

    constructor_.Reset(tpl->GetFunction());
    exports->Set(Nan::New("RSConfig").ToLocalChecked(), tpl->GetFunction());
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
  RSConfig() : config_(nullptr), error_(nullptr) {}

  ~RSConfig() {
    DestroyMe();
  }

  void DestroyMe() {
    if (error_) rs2_free_error(error_);
    error_ = nullptr;
    if (config_) rs2_delete_config(config_);
    config_ = nullptr;
  }

  static NAN_METHOD(Destroy) {
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (me) me->DestroyMe();
    info.GetReturnValue().Set(Nan::Undefined());
  }

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.IsConstructCall()) {
      RSConfig* obj = new RSConfig();
      obj->config_ = rs2_create_config(&obj->error_);
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }

  // TODO(halton): added all the overloads
  static NAN_METHOD(EnableStream) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    auto stream = info[0]->IntegerValue();
    auto index = info[1]->IntegerValue();
    auto width = info[2]->IntegerValue();
    auto height = info[3]->IntegerValue();
    auto format = info[4]->IntegerValue();
    auto framerate = info[5]->IntegerValue();
    if (!me || !me->config_) return;

    CallNativeFunc(rs2_config_enable_stream, &me->error_, me->config_,
      (rs2_stream)stream,
      index,
      width,
      height,
      (rs2_format)format,
      framerate,
      &me->error_);
  }

  static NAN_METHOD(EnableAllStreams) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (!me) return;

    CallNativeFunc(rs2_config_enable_all_stream, &me->error_, me->config_,
        &me->error_);
  }

  static NAN_METHOD(EnableDevice) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (!me) return;

    auto device = info[0]->ToString();
    v8::String::Utf8Value value(device);
    CallNativeFunc(rs2_config_enable_device, &me->error_, me->config_, *value,
        &me->error_);
  }

  static NAN_METHOD(EnableDeviceFromFile) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (!me) return;

    auto device_file = info[0]->ToString();
    v8::String::Utf8Value value(device_file);
    CallNativeFunc(rs2_config_enable_device_from_file, &me->error_, me->config_,
        *value, &me->error_);
  }

  static NAN_METHOD(EnableDeviceFromFileRepeatOption) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (!me) return;

    auto device_file = info[0]->ToString();
    auto repeat = info[1]->BooleanValue();
    v8::String::Utf8Value value(device_file);
    CallNativeFunc(rs2_config_enable_device_from_file_repeat_option,
        &me->error_, me->config_, *value, repeat, &me->error_);
  }

  static NAN_METHOD(EnableRecordToFile) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (!me) return;

    auto device_file = info[0]->ToString();
    v8::String::Utf8Value value(device_file);
    CallNativeFunc(rs2_config_enable_record_to_file, &me->error_, me->config_,
        *value, &me->error_);
  }

  static NAN_METHOD(DisableStream) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (!me) return;

    auto stream = info[0]->IntegerValue();
    CallNativeFunc(rs2_config_disable_stream, &me->error_, me->config_,
        (rs2_stream)stream, &me->error_);
  }

  static NAN_METHOD(DisableAllStreams) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSConfig>(info.Holder());
    if (!me) return;

    CallNativeFunc(rs2_config_disable_all_streams, &me->error_, me->config_,
        &me->error_);
  }
  static NAN_METHOD(Resolve);
  static NAN_METHOD(CanResolve);

 private:
  static Nan::Persistent<v8::Function> constructor_;
  friend class RSPipeline;

  rs2_config* config_;
  rs2_error* error_;
};

Nan::Persistent<v8::Function> RSConfig::constructor_;

#endif
