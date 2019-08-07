#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

class RSFrameQueue : public Nan::ObjectWrap {
public:
  static void Init(v8::Local<v8::Object> exports) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RSFrameQueue").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "create", Create);
    Nan::SetPrototypeMethod(tpl, "destroy", Destroy);
    Nan::SetPrototypeMethod(tpl, "waitForFrame", WaitForFrame);
    Nan::SetPrototypeMethod(tpl, "pollForFrame", PollForFrame);
    Nan::SetPrototypeMethod(tpl, "enqueueFrame", EnqueueFrame);

    constructor_.Reset(tpl->GetFunction());
    exports->Set(Nan::New("RSFrameQueue").ToLocalChecked(), tpl->GetFunction());
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
  RSFrameQueue() : frame_queue_(nullptr), error_(nullptr) {}

  ~RSFrameQueue() { DestroyMe(); }

  void DestroyMe() {
    if (error_)
      rs2_free_error(error_);
    error_ = nullptr;
    if (frame_queue_)
      rs2_delete_frame_queue(frame_queue_);
    frame_queue_ = nullptr;
  }

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
    if (info.IsConstructCall()) {
      RSFrameQueue *obj = new RSFrameQueue();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }

  static NAN_METHOD(WaitForFrame) {
    info.GetReturnValue().Set(Nan::Undefined());
    int32_t timeout = info[0]->IntegerValue(); // in ms
    auto me = Nan::ObjectWrap::Unwrap<RSFrameQueue>(info.Holder());
    if (!me)
      return;

    auto frame =
        GetNativeResult<rs2_frame *>(rs2_wait_for_frame, &me->error_,
                                     me->frame_queue_, timeout, &me->error_);
    if (!frame)
      return;

    info.GetReturnValue().Set(RSFrame::NewInstance(frame));
  }

  static NAN_METHOD(Create) {
    info.GetReturnValue().Set(Nan::Undefined());
    int32_t capacity = info[0]->IntegerValue();
    auto me = Nan::ObjectWrap::Unwrap<RSFrameQueue>(info.Holder());
    if (!me)
      return;

    me->frame_queue_ = GetNativeResult<rs2_frame_queue *>(
        rs2_create_frame_queue, &me->error_, capacity, &me->error_);
  }

  static NAN_METHOD(Destroy) {
    auto me = Nan::ObjectWrap::Unwrap<RSFrameQueue>(info.Holder());
    if (me) {
      me->DestroyMe();
    }
    info.GetReturnValue().Set(Nan::Undefined());
  }

  static NAN_METHOD(PollForFrame) {
    info.GetReturnValue().Set(Nan::Undefined());
    auto me = Nan::ObjectWrap::Unwrap<RSFrameQueue>(info.Holder());
    if (!me)
      return;

    rs2_frame *frame = nullptr;
    auto res = GetNativeResult<int>(rs2_poll_for_frame, &me->error_,
                                    me->frame_queue_, &frame, &me->error_);
    if (!res)
      return;

    info.GetReturnValue().Set(RSFrame::NewInstance(frame));
  }

  static NAN_METHOD(EnqueueFrame) {
    auto me = Nan::ObjectWrap::Unwrap<RSFrameQueue>(info.Holder());
    auto frame = Nan::ObjectWrap::Unwrap<RSFrame>(info[0]->ToObject());

    if (me && frame) {
      rs2_enqueue_frame(frame->frame_, me->frame_queue_);
      frame->frame_ = nullptr;
    }
    info.GetReturnValue().Set(Nan::Undefined());
  }

private:
  static Nan::Persistent<v8::Function> constructor_;
  rs2_frame_queue *frame_queue_;
  rs2_error *error_;
  friend class RSDevice;
};

Nan::Persistent<v8::Function> RSFrameQueue::constructor_;


#endif
