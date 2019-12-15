#ifndef NOTIFICATION_CALLBACKS_H
#define NOTIFICATION_CALLBACKS_H

#include "napi-thread-safe-callback.hpp"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>

class NotificationCallback : public rs2_notifications_callback {
  private:
	std::shared_ptr<ThreadSafeCallback> fn_;

  public:
	explicit NotificationCallback(std::shared_ptr<ThreadSafeCallback> fn)
      : fn_(fn) {
	}
	void on_notification(rs2_notification *notification) override {
		if (!notification) return;

		this->fn_->call([notification](Napi::Env env, std::vector<napi_value>& args) {
            rs2_error *error_ = nullptr;

            const char *desc
            = GetNativeResult<const char *>(rs2_get_notification_description, &error_, notification, &error_);
            auto time = GetNativeResult<rs2_time_t>(rs2_get_notification_timestamp, &error_, notification, &error_);
            auto severity
            = GetNativeResult<rs2_log_severity>(rs2_get_notification_severity, &error_, notification, &error_);
            auto category
            = GetNativeResult<rs2_notification_category>(rs2_get_notification_category, &error_, notification, &error_);
            auto serialized_data
            = GetNativeResult<std::string>(rs2_get_notification_serialized_data, &error_, notification, &error_);

            auto notification_obj = new RSNotification(env, desc, time, severity, category, serialized_data);

            args = { notification_obj->GetObject() };
        });
	}
	void release() override {
		delete this;
	}
};

#endif
