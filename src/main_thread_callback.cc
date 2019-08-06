#ifndef MAINTHREADCALLBACK_H
#define MAINTHREADCALLBACK_H

#include <iostream>
#include <list>
#include <napi.h>
#include <uv.h>

using namespace std;

class MainThreadCallbackInfo {
  public:
	MainThreadCallbackInfo()
	  : consumed_(false) {
		pending_infos_.push_back(this);
	}
	virtual ~MainThreadCallbackInfo() {
		pending_infos_.erase(find(pending_infos_.begin(), pending_infos_.end(), this));
	}
	virtual void Run() {
	}
	virtual void Release() {
	}
	void SetConsumed() {
		consumed_ = true;
	}
	static bool InfoExist(MainThreadCallbackInfo* info) {
		auto result = find(pending_infos_.begin(), pending_infos_.end(), info);
		return (result != pending_infos_.end());
	}
	static void ReleasePendingInfos() {
		while (pending_infos_.size()) { delete *(pending_infos_.begin()); }
	}

  protected:
	static list<MainThreadCallbackInfo*> pending_infos_;
	bool consumed_;
};

list<MainThreadCallbackInfo*> MainThreadCallbackInfo::pending_infos_;

class MainThreadCallback {
  public:
	static void Init() {
		if (!singleton_) singleton_ = new MainThreadCallback();
	}
	static void Destroy() {
		if (singleton_) {
			delete singleton_;
			singleton_ = nullptr;
			MainThreadCallbackInfo::ReleasePendingInfos();
		}
	}
	~MainThreadCallback() {
		uv_close(reinterpret_cast<uv_handle_t*>(async_), [](uv_handle_t* ptr) -> void { free(ptr); });
	}
	static void NotifyMainThread(MainThreadCallbackInfo* info) {
		if (singleton_) {
			singleton_->async_->data = static_cast<void*>(info);
			uv_async_send(singleton_->async_);
		}
	}

  private:
	MainThreadCallback() {
		async_ = static_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
		uv_async_init(uv_default_loop(), async_, AsyncProc);
		// std::cerr << "MainThreadCallback inited" << std::endl;
	}

	static void AsyncProc(uv_async_t* async) {
		if (!(async->data)) return;

		MainThreadCallbackInfo* info = reinterpret_cast<MainThreadCallbackInfo*>(async->data);
		info->Run();
		// As the above info->Run() enters js world and during that, any code
		// such as cleanup() could be called to release everything. So this info
		// may has been released, we need to check before releasing it.
		if (MainThreadCallbackInfo::InfoExist(info)) delete info;
	}
	static MainThreadCallback* singleton_;

	uv_async_t* async_;
};

MainThreadCallback* MainThreadCallback::singleton_ = nullptr;

#endif
