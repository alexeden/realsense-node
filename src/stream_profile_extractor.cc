#ifndef STREAM_PROFILE_EXTRACTOR_H
#define STREAM_PROFILE_EXTRACTOR_H

#include <iostream>
#include <librealsense2/hpp/rs_types.hpp>

class StreamProfileExtractor {
  public:
	explicit StreamProfileExtractor(const rs2_stream_profile* profile) {
		rs2_get_stream_profile_data(profile, &stream_, &format_, &index_, &unique_id_, &fps_, &error_);
	}
	~StreamProfileExtractor() {
	}
	rs2_stream stream_;
	rs2_format format_;
	int32_t fps_;
	int32_t index_;
	int32_t unique_id_;
	rs2_error* error_;
};

#endif
