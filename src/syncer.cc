#ifndef SYNCER_H
#define SYNCER_H

class RSSyncer : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RSSyncer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
    Nan::SetPrototypeMethod(tpl, "waitForFrames", WaitForFrames);
    Nan::SetPrototypeMethod(tpl, "pollForFrames", PollForFrames);

    constructor_.Reset(tpl->GetFunction());
    exports->Set(Nan::New("RSSyncer").ToLocalChecked(), tpl->GetFunction());
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
  RSSyncer() : syncer_(nullptr), frame_queue_(nullptr), error_(nullptr) {}

  ~RSSyncer() {
    DestroyMe();
  }

  void DestroyMe() {
    if (error_) rs2_free_error(error_);
    error_ = nullptr;
    if (syncer_) rs2_delete_processing_block(syncer_);
    syncer_ = nullptr;
    if (frame_queue_) rs2_delete_frame_queue(frame_queue_);
    frame_queue_ = nullptr;
  }

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.IsConstructCall()) {
      RSSyncer* obj = new RSSyncer();
      obj->syncer_ = GetNativeResult<rs2_processing_block*>(
          rs2_create_sync_processing_block, &obj->error_, &obj->error_);
      obj->frame_queue_ = GetNativeResult<rs2_frame_queue*>(
          rs2_create_frame_queue, &obj->error_, 1, &obj->error_);
      auto callback = new FrameCallbackForFrameQueue(obj->frame_queue_);
      CallNativeFunc(rs2_start_processing, &obj->error_, obj->syncer_, callback,
          &obj->error_);
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }

  static NAN_METHOD(WaitForFrames) {
    info.GetReturnValue().Set(Nan::False());
    auto me = Nan::ObjectWrap::Unwrap<RSSyncer>(info.Holder());
    auto frameset = Nan::ObjectWrap::Unwrap<RSFrameSet>(info[0]->ToObject());
    auto timeout = info[1]->IntegerValue();
    if (!me || !frameset) return;

    rs2_frame* frames = GetNativeResult<rs2_frame*>(rs2_wait_for_frame,
        &me->error_, me->frame_queue_, timeout, &me->error_);
    if (!frames) return;

    frameset->Replace(frames);
    info.GetReturnValue().Set(Nan::True());
  }

  static NAN_METHOD(Destroy) {
    auto me = Nan::ObjectWrap::Unwrap<RSSyncer>(info.Holder());
    if (me) {
      me->DestroyMe();
    }
    info.GetReturnValue().Set(Nan::Undefined());
  }

  static NAN_METHOD(PollForFrames) {
    info.GetReturnValue().Set(Nan::False());
    auto me = Nan::ObjectWrap::Unwrap<RSSyncer>(info.Holder());
    auto frameset = Nan::ObjectWrap::Unwrap<RSFrameSet>(info[0]->ToObject());
    if (!me || !frameset) return;

    rs2_frame* frame_ref = nullptr;
    auto res = GetNativeResult<int>(rs2_poll_for_frame, &me->error_,
        me->frame_queue_, &frame_ref, &me->error_);
    if (!res) return;

    frameset->Replace(frame_ref);
    info.GetReturnValue().Set(Nan::True());
  }

 private:
  static Nan::Persistent<v8::Function> constructor_;
  rs2_processing_block* syncer_;
  rs2_frame_queue* frame_queue_;
  rs2_error* error_;
  friend class RSSensor;
};

Nan::Persistent<v8::Function> RSSyncer::constructor_;

#endif
