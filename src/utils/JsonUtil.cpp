#include "JsonUtil.h"

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message_fwd.hpp>

#include <string>

HttpResponse JsonUtil::buildErrorResponse(const http::status& status, unsigned int version,
										  const std::string& message) {
	HttpResponse res{status, version};
	res.set(http::field::content_type, "application/json");
	res.set(http::field::server, "MusicPlayer-BackEnd");

	res.body() = json{{"error", message}}.dump();
	res.prepare_payload();

	return res;
}

HttpResponse JsonUtil::buildSuccessResponse(unsigned int version, const std::string& json_msg) {
	HttpResponse res{http::status::ok, version};
	res.set(http::field::content_type, "application/json");
	res.set(http::field::server, "MusicPlayer-BackEnd");

	res.body() = json_msg;
	res.prepare_payload();

	return res;
}
