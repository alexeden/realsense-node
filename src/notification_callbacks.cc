#ifndef NOTIFICATION_CALLBACKS_H
#define NOTIFICATION_CALLBACKS_H

#include "main_thread_callback.cc"
#include "sensor.cc"
#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>

class RSDevice;
class RSSensor;

class NotificationCallbackInfo : public MainThreadCallbackInfo {
  public:
	NotificationCallbackInfo(
	  const char *desc,
	  rs2_time_t time,
	  rs2_log_severity severity,
	  rs2_notification_category category,
	  std::string serialized_data,
	  RSSensor *s)
	  : desc_(desc)
	  , time_(time)
	  , severity_(severity)
	  , category_(category)
	  , serialized_data_(serialized_data)
	  , sensor_(s) {
	}
	virtual ~NotificationCallbackInfo() {
	}
	virtual void Run() {
	}

//   private:
	std::string desc_;
	rs2_time_t time_;
	rs2_log_severity severity_;
	rs2_notification_category category_;
	std::string serialized_data_;
	RSSensor *sensor_;
	rs2_error *error_;
};

class NotificationCallback : public rs2_notifications_callback {
  public:
	explicit NotificationCallback(RSSensor *s)
	  : error_(nullptr)
	  , sensor_(s) {
	}
	void on_notification(rs2_notification *notification) override {
		if (notification) {
			const char *desc
			  = GetNativeResult<const char *>(rs2_get_notification_description, &error_, notification, &error_);
			rs2_time_t time
			  = GetNativeResult<rs2_time_t>(rs2_get_notification_timestamp, &error_, notification, &error_);
			rs2_log_severity severity
			  = GetNativeResult<rs2_log_severity>(rs2_get_notification_severity, &error_, notification, &error_);
			rs2_notification_category category = GetNativeResult<
			  rs2_notification_category>(rs2_get_notification_category, &error_, notification, &error_);
			std::string serialized_data
			  = GetNativeResult<std::string>(rs2_get_notification_serialized_data, &error_, notification, &error_);
			MainThreadCallback::NotifyMainThread(
			  new NotificationCallbackInfo(desc, time, severity, category, serialized_data, sensor_));
		}
	}
	void release() override {
		delete this;
	}
	rs2_error *error_;
	RSSensor *sensor_;
};

#endif
