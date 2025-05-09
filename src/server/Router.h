#pragma once
#include "../common/net.h"

#include <map>

// 路由回调类型
using RouterHandler = std::function<HttpResponse(const HttpRequest&)>;

class Router {
public:
	// 添加路由
	void addRouter(const http::verb& method, const std::string& url, RouterHandler handler);

	// 处理路由
	HttpResponse handleRouter(const HttpRequest& request);

private:
	std::map<std::string, RouterHandler> m_router_get;
	std::map<std::string, RouterHandler> m_router_post;
};