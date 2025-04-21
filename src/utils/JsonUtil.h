#pragma once
#include "../common/net.h"

#include <boost/beast/http/status.hpp>
#include <string>

class JsonUtil {
public:
	static HttpResponse buildErrorResponse(const http::status& status, unsigned int version,
										   const std::string& message = "Internal Server Error");

	static HttpResponse buildSuccessResponse(unsigned int version,
											 const std::string& message = "OK");
};