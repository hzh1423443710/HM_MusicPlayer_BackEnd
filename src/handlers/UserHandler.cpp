#include "UserHandler.h"
#include "../utils/JsonUtil.h"
#include "../utils/PasswordUtil.h"
#include "../server/VerifyService.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <string>
#include <regex>
#include <filesystem>
#include <fstream>

constexpr const char* TAG = "[UserHandler]";

namespace fs = std::filesystem;

UserHandler::UserHandler(const std::string& jwt_secret) : jwt_util{jwt_secret} {}

HttpResponse UserHandler::handleGetVerifyCode(const HttpRequest& req) {
	try {
		json j = json::parse(req.body());
		if (!j.contains("email") || !j.contains("action")) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Missing email or action");
		}

		std::string email = j["email"];
		std::string action = j["action"];
		if (!isValidEmail(email)) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Invalid email format");
		}

		if (action == "register" || action == "reset_password") {
			// 发送验证码到注册邮箱
			if (VerifyService::getInstance()->sendVerifyCode(email, action)) {
				return JsonUtil::buildSuccessResponse(
					req.version(),
					json{{"code", 200}, {"message", "Verification code sent"}}.dump());
			}

			return JsonUtil::buildErrorResponse(http::status::internal_server_error, req.version(),
												"Failed to send verification code");
		}

		return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
											"Invalid action");
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

HttpResponse UserHandler::handleRegister(const HttpRequest& req) {
	try {
		json j = json::parse(req.body());

		// 提取参数
		if (!j.contains("username") || !j.contains("password") || !j.contains("email") ||
			!j.contains("verify_code")) {
			return JsonUtil::buildErrorResponse(
				http::status::bad_request, req.version(),
				"Missing username or password or email or verify_code");
		}

		std::string username = j["username"];
		std::string password = j["password"];
		std::string email = j["email"];
		std::string verify_code = j["verify_code"];

		if (!VerifyService::getInstance()->verifyCode(email, "register", verify_code)) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Invalid verification code");
		}

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
		if (!PasswordUtil::verifyPassword(password, user->passwd_hash)) {
			return JsonUtil::buildErrorResponse(http::status::unauthorized, req.version(),
												"Incorrect password");
		}

		// 生成JWT, 120天有效
		std::string token = jwt_util.generateToken(
			user->username, {{"id", std::to_string(user->id)}, {"email", user->email}},
			std::chrono::hours{24 * 120});

		// clang-format off
        json body = {
            {"code", 200},
            {"message", "Login successful"}, 
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

		// Token 存储在响应头
		HttpResponse res = JsonUtil::buildSuccessResponse(req.version(), body.dump());
		res.set("Authorization", token);

		return res;
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

HttpResponse UserHandler::handleGetProfile(const HttpRequest& req) {
	try {
		const std::string token = this->jwt_util.verifyToken( req["Authorization"]);
		if (token.empty()) {
			return JsonUtil::buildErrorResponse(http::status::unauthorized, req.version(),
												"Invalid token");
		}

		int user_id = atoi(JWTUtil::getClaim(token, "id").c_str());

		auto user = user_dao.getUserById(user_id);
		if (!user) {
			return JsonUtil::buildErrorResponse(http::status::not_found, req.version(),
												"User not found");
		}

		json response = {
			{"code", 200},
			{"message", "User profile retrieved successfully"},
			{"user", {
				{"id", user->id},
				{"username", user->username},
				{"email", user->email},
				{"create_at", user->create_at},
				{"qq_id", user->qq_id},
				{"netease_id", user->netease_id}
			}}
		};

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

HttpResponse UserHandler::handleUpdateAvatar(const HttpRequest& req) {
	try {
		const std::string token = this->jwt_util.verifyToken( req["Authorization"]);
		if (token.empty()) {
			return JsonUtil::buildErrorResponse(http::status::unauthorized, req.version(),
												"Invalid token");
		}

		json j = json::parse(req.body());
		if (!j.contains("avatar_data")) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Missing avatar_data");
		}

		std::string avatar_data = j["avatar_data"];
		if (avatar_data.size() > 1024 * 1024) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Avatar data requires not more than 1MB");
		}

		int user_id = atoi(JWTUtil::getClaim(token, "id").c_str());

		// 创建用户头像目录
		if (!fs::exists(UserHandler::AVATAR_PATH)) {
			fs::create_directory(UserHandler::AVATAR_PATH);
		}

		std::string avatar_path = UserHandler::AVATAR_PATH + "/" + std::to_string(user_id);

		std::ofstream avatar_file(avatar_path, std::ios::binary);
		if (!avatar_file) {
			return JsonUtil::buildErrorResponse(http::status::internal_server_error, req.version(),
												"Failed to save avatar");
		}
		avatar_file << avatar_data;
		avatar_file.close();

		return JsonUtil::buildSuccessResponse(req.version(),
											  json{{"code", 200}, {"message", "Avatar updated successfully"}}.dump());	
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

HttpResponse UserHandler::handleGetAvatar(const HttpRequest& req) {
	try {
		const std::string token = this->jwt_util.verifyToken( req["Authorization"]);
		if (token.empty()) {
			return JsonUtil::buildErrorResponse(http::status::unauthorized, req.version(),
												"Invalid token");
		}

		int user_id = atoi(JWTUtil::getClaim(token, "id").c_str());
		std::string avatar_path = UserHandler::AVATAR_PATH + "/" + std::to_string(user_id);
		if (!fs::exists(avatar_path)) {
			return JsonUtil::buildErrorResponse(http::status::not_found, req.version(),
												"Avatar not found");
		}

		std::ifstream avatar_file(avatar_path, std::ios::binary);
		if (!avatar_file) {
			return JsonUtil::buildErrorResponse(http::status::internal_server_error, req.version(),
												"Failed to read avatar");
		}
		std::string avatar_data((std::istreambuf_iterator<char>(avatar_file)),
								std::istreambuf_iterator<char>());
		avatar_file.close();
		json response = {
			{"code", 200},
			{"message", "Avatar retrieved successfully"},
			{"avatar_data", avatar_data}
		};

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


HttpResponse UserHandler::handleChangePassword(const HttpRequest& req) {
	try {
		json j = json::parse(req.body());

		if ( !j.contains("new_password") || !j.contains("verify_code") || !j.contains("email")) { 
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Missing user_id or new_password or verify_code or email");
		}

		const std::string token = this->jwt_util.verifyToken( req["Authorization"]);
		if (token.empty()) {
			return JsonUtil::buildErrorResponse(http::status::unauthorized, req.version(),
												"Invalid token");
		}

		int user_id = atoi(JWTUtil::getClaim(token, "id").c_str());
		std::string new_password = j["new_password"];
		std::string verify_code = j["verify_code"];
		std::string email = j["email"];

		if (!VerifyService::getInstance()->verifyCode(email, "reset_password", verify_code)) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Invalid verification code");
		}

		if (!user_dao.updatePassword(user_id,new_password))
			return JsonUtil::buildErrorResponse(http::status::internal_server_error,
												req.version(), "Failed to update password");

		return JsonUtil::buildSuccessResponse(req.version(),
											  json{{"code", 200}, {"message", "Password changed successfully"}}.dump());
	
	} catch (const json::exception& e) {
		spdlog::error("{} JSON parse error in {}: {}", TAG, __FUNCTION__, e.what());
		return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
											"Invalid JSON format");
	} catch (const std::exception& e) {
		spdlog::error("{} Exception in {}: {}", TAG, __FUNCTION__, e.what());
		return JsonUtil::buildErrorResponse(http::status::internal_server_error, req.version(),
											"Internal server error");
	}
	return {};
	
}

HttpResponse UserHandler::handleBindPlatform(const HttpRequest& req) {
	try {
		const std::string token = this->jwt_util.verifyToken( req["Authorization"]);
		if (token.empty()) {
			return JsonUtil::buildErrorResponse(http::status::unauthorized, req.version(),
												"Invalid token");
		}

		int user_id = atoi(JWTUtil::getClaim(token, "id").c_str());
		json j = json::parse(req.body());
		if (!j.contains("platform") || !j.contains("platform_id")) {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Missing platform or platform_id");
		}

		const std::string platform = j["platform"];
		const std::string platform_id = j["platform_id"];
		if (platform == "qq") {
			if (!user_dao.updateQQId(user_id, platform_id)) {
				return JsonUtil::buildErrorResponse(http::status::internal_server_error,
													req.version(), "Failed to bind QQ account");
			}
		} else if (platform == "netease") {
			if (!user_dao.updateNetEaseId(user_id, platform_id)) {
				return JsonUtil::buildErrorResponse(http::status::internal_server_error,
													req.version(), "Failed to bind NetEase account");
			}
		} else {
			return JsonUtil::buildErrorResponse(http::status::bad_request, req.version(),
												"Invalid platform");
		}

		json response = {
			{"code", 200},
			{"message", "Platform account bound successfully"}
		};

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

bool UserHandler::isValidEmail(const std::string& email) {
	std::regex email_regex(
		R"((^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$))");
	return std::regex_match(email, email_regex);
}
