#pragma once
#include <string>

#include "../database//UserDAO.h"
#include "../utils/JWTUtil.h"
#include "../common/net.h"

class UserHandler {
public:
	UserHandler(const std::string& jwt_secret);

	// 获取验证码
	HttpResponse handleGetVerifyCode(const HttpRequest& req);

	/**
	 * @brief 用户注册
	 */
	HttpResponse handleRegister(const HttpRequest& req);

	/**
	 * @brief 用户登录
	 * @param req HTTP请求
	 * @return HTTP响应(Header: Authorization: JWT-Token)
	 */
	HttpResponse handleLogin(const HttpRequest& req);

	/**
	 * @brief 获取用户信息(JWT-Token)
	 * @param req HTTP请求
	 * @return HTTP响应
	 */
	HttpResponse handleGetProfile(const HttpRequest& req);

	/**
	 * @brief 更新用户信息(JWT-Token)
	 * @param req HTTP请求
	 * @return HTTP响应
	 */
	HttpResponse handleUpdateProfile(const HttpRequest& req);

	/**
	 * @brief 重置密码(JWT-Token)
	 * @param req HTTP请求
	 * @return HTTP响应
	 */
	HttpResponse handleChangePassword(const HttpRequest& req);

	/**
	 * @brief 绑定第三方平台账号(JWT-Token)
	 * @param req HTTP请求
	 * @return HTTP响应
	 */
	HttpResponse handleBindPlatform(const HttpRequest& req);

private:
	// 验证邮箱格式
	static bool isValidEmail(const std::string& email);

private:
	UserDAO user_dao;
	JWTUtil jwt_util;
};