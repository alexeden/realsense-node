#ifndef FRAME_CALLBACKS_H
#define FRAME_CALLBACKS_H

#include "napi-thread-safe-callback.hpp"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>

// class RSSensor;
// class FrameCallbackInfo : public MainThreadCallbackInfo {
//   public:
// 	FrameCallbackInfo(rs2_frame* frame, void* data)
// 	  : frame_(frame)
// 	  , sensor_(static_cast<RSSensor*>(data)) {
// 	}
// 	virtual ~FrameCallbackInfo() {
// 		if (!consumed_) Release();
// 	}
// 	virtual void Run() {
// 	}
// 	virtual void Release() {
// 		if (frame_) {
// 			rs2_release_frame(frame_);
// 			frame_ = nullptr;
// 		}
// 	}

//   private:
// 	rs2_frame* frame_;
// 	RSSensor* sensor_;
// };

class FrameCallbackForProc : public rs2_frame_callback {
  private:
	std::shared_ptr<ThreadSafeCallback> fn_;
	void* callback_data_;

  public:
	explicit FrameCallbackForProc(std::shared_ptr<ThreadSafeCallback> fn, void* data)
	  : fn_(fn)
	  , callback_data_(data) {
		std::cerr << "Constructing FrameCallbackForProc" << std::endl;
	}
	void on_frame(rs2_frame* frame) override {
		this->fn_->call([frame](Napi::Env env, std::vector<napi_value>& args) {
			std::cerr << __FILE__ << ":" << __LINE__ << "\tFrameCallbackForProc::on_frame" << std::endl;
			throw "Not implemented!";
		});
	}
	void release() override {
		delete this;
	}
};

class FrameCallbackForFrameQueue : public rs2_frame_callback {
  public:
	explicit FrameCallbackForFrameQueue(rs2_frame_queue* queue)
	  : frame_queue_(queue) {
	}
	void on_frame(rs2_frame* frame) override {
		if (frame && frame_queue_) rs2_enqueue_frame(frame, frame_queue_);
	}
	void release() override {
		delete this;
	}
	rs2_frame_queue* frame_queue_;
};

class FrameCallbackForProcessingBlock : public rs2_frame_callback {
  public:
	explicit FrameCallbackForProcessingBlock(rs2_processing_block* block_ptr)
	  : block_(block_ptr)
	  , error_(nullptr) {
		std::cerr << "Constructing FrameCallbackForProcessingBlock" << std::endl;
	}
	virtual ~FrameCallbackForProcessingBlock() {
		if (error_) rs2_free_error(error_);
	}
	void on_frame(rs2_frame* frame) override {
		rs2_process_frame(block_, frame, &error_);
	}
	void release() override {
		delete this;
	}
	rs2_processing_block* block_;
	rs2_error* error_;
};

#endif
