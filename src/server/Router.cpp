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

void Router::addRouter(const http::verb& method, const std::string& path, RouterHandler handler) {
	auto [regex, path_params] = pathToRegex(path);
	RouterEntry entry;
	entry.method = method;
	entry.path_regex = regex;
	entry.path_params = std::move(path_params);
	entry.handler = std::move(handler);

	m_routers.emplace_back(std::move(entry));
}

HttpResponse Router::router(const HttpRequest& request) {
	HttpRequest req_copy = request;

	for (const auto& route : m_routers) {
		if (route.method != req_copy.method()) continue;

		std::string target = req_copy.target();
		std::smatch match;

		if (std::regex_match(target, match, route.path_regex)) {
			// 解析路径参数
			parsePathParams(req_copy, match, route.path_params);

			try {
				// 回调处理
				return route.handler(req_copy);
			} catch (const std::exception& e) {
				spdlog::error("{} Route handler: {}", TAG, e.what());
				return JsonUtil::buildErrorResponse(http::status::internal_server_error,
													req_copy.version());
			}
		}
	}

	return JsonUtil::buildErrorResponse(http::status::not_found, req_copy.version(), "Not found");
}

std::pair<std::regex, std::vector<std::string>> Router::pathToRegex(const std::string& path) {
	std::string pattern = path;
	std::vector<std::string> param_names;

	// 将路径中的 :param 替换为正则表达式捕获组
	std::regex param_regex(":(\\w+)");
	std::string replaced = std::regex_replace(pattern, param_regex, "([^/]+)");

	// 提取参数名
	std::sregex_iterator it(pattern.begin(), pattern.end(), param_regex);
	std::sregex_iterator end;

	while (it != end) {
		param_names.push_back((*it)[1].str());
		++it;
	}

	// 处理末尾的可选斜杠
	if (!replaced.empty() && replaced.back() == '/') {
		replaced.pop_back();
		replaced += "/?";
	} else {
		replaced += "/?";
	}

	// 添加开始和结束锚点
	replaced = "^" + replaced + "$";

	return {std::regex(replaced), param_names};
}

void Router::parsePathParams(http::request<http::string_body>& request, const std::smatch& matches,
							 const std::vector<std::string>& param_names) {
	// 在Beast中，我们不能直接添加属性，所以将参数存储在header中
	for (size_t i = 0; i < param_names.size() && i + 1 < matches.size(); ++i) {
		// 使用特殊前缀以避免与正常header冲突
		request.set("X-Path-Param-" + param_names[i], matches[i + 1].str());
	}
}
