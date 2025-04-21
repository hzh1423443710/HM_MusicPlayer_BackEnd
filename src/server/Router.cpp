#include "Router.h"
#include "../utils/JsonUtil.h"

#include <boost/beast/http/status.hpp>
#include <spdlog/spdlog.h>
#include <boost/beast/http/verb.hpp>

#include <regex>
#include <string>
#include <utility>
#include <vector>

constexpr const char* TAG = "[Router]";

void Router::addRouter(const http::verb& method, const std::regex& path_regex,
					   RouterHandler handler) {
	RouterEntry entry;
	entry.method = method;
	entry.path_regex = path_regex;
	entry.handler = std::move(handler);
	m_routers.emplace_back(std::move(entry));
}

HttpResponse Router::handleRouter(const HttpRequest& request) {
	for (const auto& route : m_routers) {
		if (route.method != request.method()) continue;

		// TODO: 调用request.target.data()会乱码
		if (!std::regex_match(std::string(request.target()), route.path_regex)) continue;

		spdlog::info("{} {} {}", TAG, request.method_string(), request.target());
		return route.handler(request);
	}

	return JsonUtil::buildErrorResponse(http::status::not_found, request.version(), "Not found");
}
