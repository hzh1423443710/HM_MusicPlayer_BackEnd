#pragma once
#include "../common/net.h"

#include <regex>
#include <string>
#include <utility>
#include <vector>

// 路由回调类型
using RouterHandler = std::function<HttpResponse(const HttpRequest&)>;

struct RouterEntry {
	http::verb method;					  // 请求方法
	std::regex path_regex;				  // 路径正则
	std::vector<std::string> path_params; // 路径参数
	RouterHandler handler;				  // 处理函数
};

class Router {
public:
	Router() = default;

	// 添加路由
	void addRouter(const http::verb& method, const std::string& path, RouterHandler handler);

	// 处理路由
	HttpResponse router(const HttpRequest& request);

private:
	// 解析路径参数 :parm
	static std::pair<std::regex, std::vector<std::string>> pathToRegex(const std::string& path);

	// 解析路径参数
	static void parsePathParams(HttpRequest& request, const std::smatch& match,
								const std::vector<std::string>& params);

private:
	std::vector<RouterEntry> m_routers;
};