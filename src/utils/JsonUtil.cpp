#include "JsonUtil.h"

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/url.hpp>

#include <boost/url/url_view.hpp>
#include <map>
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

std::vector<std::string> JsonUtil::getPathParams(std::string_view path) {
	auto segs = boost::urls::url_view(path).segments();
	std::vector<std::string> params;
	params.reserve(segs.size());
	for (auto seg : segs) {
		params.emplace_back(seg);
	}

	return params;
}

std::map<std::string, std::string> JsonUtil::getQueryParams(std::string_view query) {
	auto params = boost::urls::url_view(query).params();
	std::map<std::string, std::string> queryParams;

	for (auto param : params) {
		queryParams.insert({param.key, param.value});
	}

	return queryParams;
}
