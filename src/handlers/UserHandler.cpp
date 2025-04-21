#include "UserHandler.h"
#include "../utils/JsonUtil.h"
#include "../utils/PasswordUtil.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <string>

constexpr const char* TAG = "[UserHandler]";

UserHandler::UserHandler(const std::string& jwt_secret) : jwt_util{jwt_secret} {}

HttpResponse UserHandler::handleRegister(const HttpRequest& req) {
	try {
		json j = json::parse(req.body());

		// 提取参数
		if (!j.contains("username") || !j.contains("password") || !j.contains("email")) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Missing username or password or email");
		}

		std::string username = j["username"];
		std::string password = j["password"];
		std::string email = j["email"];

		// 检查用户是否已存在
		if (user_dao.getUserByUsernameOrEmail(username)) {
			return JsonUtil::buildErrorResponse(http::status::conflict, req.version(),
												"Username already exists");
		}

		User user;
		user.username = username;
		user.passwd_hash = PasswordUtil::hashPassword(password);
		user.email = email;

		if (!user_dao.createUser(user)) {
			return JsonUtil::buildErrorResponse(http::status::internal_server_error, req.version(),
												"Failed to create user");
		}

		json response = {
			{"code", 200}, {"message", "User registered successfully"}, {"user_id", user.id}};

		return JsonUtil::buildSuccessResponse(req.version(), response.dump());
	} catch (const json::exception& e) {
		spdlog::error("{} JSON parse error in {}: {}", TAG, __FUNCTION__, e.what());
		return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
											"Invalid JSON format");
	} catch (const std::exception& e) {
		spdlog::error("{} Exception in {}: {}", TAG, __FUNCTION__, e.what());
		return JsonUtil::buildErrorResponse(http::status::internal_server_error, req.version(),
											"Internal server error");
	}
}

HttpResponse UserHandler::handleLogin(const HttpRequest& req) {
	try {
		json j = json::parse(req.body());

		if (!j.contains("username_or_email") || !j.contains("password")) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Missing username or email or password");
		}

		std::string username_or_email = j["username_or_email"];
		std::string password = j["password"];

		// 检查用户是否存在
		auto user = user_dao.getUserByUsernameOrEmail(username_or_email);
		if (!user) {
			return JsonUtil::buildErrorResponse(http::status::not_found, req.version(),
												"User not found");
		}

		// 验证密码
		if (!PasswordUtil::verifyPassword(PasswordUtil::hashPassword(password),
										  user->passwd_hash)) {
			return JsonUtil::buildErrorResponse(http::status::unauthorized, req.version(),
												"Incorrect password");
		}

		// 生成JWT, 120天有效
		std::string token = jwt_util.generateToken(
			user->username, {{"id", std::to_string(user->id)}, {"email", user->email}},
			std::chrono::hours{24 * 120});

		// clang-format off
        json response = {
            {"code", 200},
            {"message", "Login successful"}, 
            {"token", token},
            {"user", {
                {"id", user->id},
                {"username", user->username},
                {"email", user->email},
                {"create_at", user->create_at},
                {"qq_id", user->qq_id},
                {"netease_id", user->netease_id}
            }}
        };
        // clang-format on  

        return JsonUtil::buildSuccessResponse(req.version(), response.dump());
	} catch (const json::exception& e) {
        spdlog::error("{} JSON parse error in {}: {}", TAG, __FUNCTION__, e.what());
        return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
                                            "Invalid JSON format");
	} catch (const std::exception& e) {
        spdlog::error("{} Exception in {}: {}", TAG, __FUNCTION__, e.what());
        return JsonUtil::buildErrorResponse(http::status::internal_server_error, req.version(),
                                            "Internal server error");
	}
}
