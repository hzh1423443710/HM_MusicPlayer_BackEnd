#include "Router.h"
#include "../utils/JsonUtil.h"

#include <boost/beast/http/status.hpp>
#include <spdlog/spdlog.h>
#include <boost/beast/http/verb.hpp>

#include <string>
#include <utility>

void Router::addRouter(const http::verb& method, const std::string& url, RouterHandler handler) {
	if (method == http::verb::get) {
		m_router_get[url] = std::move(handler);
		return;
	}

	if (method == http::verb::post) {
		m_router_post[url] = std::move(handler);
	}
}

HttpResponse Router::handleRouter(const HttpRequest& request) {
	const http::verb method = request.method();
	const std::string target = request.target();
	spdlog::info("{} {} {}", __FUNCTION__, request.method_string(), target);

	if (method == http::verb::get && m_router_get.find(target) != m_router_get.end()) {
		return m_router_get[target](request);
	}

	if (method == http::verb::post && m_router_post.find(target) != m_router_post.end()) {
		return m_router_post[target](request);
	}

	return JsonUtil::buildErrorResponse(http::status::not_found, request.version(), "Not found");
}
