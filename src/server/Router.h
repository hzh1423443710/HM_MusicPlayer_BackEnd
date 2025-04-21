#pragma once
#include "../common/net.h"

#include <regex>

// 路由回调类型
using RouterHandler = std::function<HttpResponse(const HttpRequest&)>;

struct RouterEntry {
	http::verb method;	   // 请求方法
	std::regex path_regex; // 路径正则
	RouterHandler handler; // 处理函数
};

class Router {
public:
	// 添加路由
	void addRouter(const http::verb& method, const std::regex& path_regex, RouterHandler handler);

	// 处理路由
	HttpResponse handleRouter(const HttpRequest& request);

private:
	std::vector<RouterEntry> m_routers;
};