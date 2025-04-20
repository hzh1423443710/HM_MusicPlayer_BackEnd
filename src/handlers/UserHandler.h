#pragma once
#include <string>

#include "../database//UserDAO.h"
#include "../utils/JWTUtil.h"
#include "../common/net.h"

class UserHandler {
public:
	UserHandler(const std::string& jwt_secret);

	// 用户注册
	HttpResponse handleRegister(const HttpRequest& req);

	// 用户登录
	HttpResponse handleLogin(const HttpRequest& req);

	// 获取用户信息
	HttpResponse handleGetProfile(const HttpRequest& req);

	// 更新用户信息
	HttpResponse handleUpdateProfile(const HttpRequest& req);

	// 修改密码
	HttpResponse handleChangePassword(const HttpRequest& req);

	// 第三方平台账号绑定
	HttpResponse handleBindPlatform(const HttpRequest& req);

private:
	UserDAO user_dao;
	JWTUtil jwt_util;

	// 从请求中提取用户ID
	bool extractUserIdFromToken(const HttpRequest& req, int& user_id);
};