#ifndef UTILS_H
#define UTILS_H

#include "error_util.cc"
#include <librealsense2/hpp/rs_types.hpp>

template<typename R, typename F, typename... arguments>
R GetNativeResult(F func, rs2_error** error, arguments... params) {
	// reset the error pointer for each call.
	*error = nullptr;
	ErrorUtil::ResetError();
	R val = func(params...);
	ErrorUtil::AnalyzeError(*error);
	return val;
}

template<typename F, typename... arguments>
void CallNativeFunc(F func, rs2_error** error, arguments... params) {
	// reset the error pointer for each call.
	*error = nullptr;
	ErrorUtil::ResetError();
	func(params...);
	ErrorUtil::AnalyzeError(*error);
}

#endif
