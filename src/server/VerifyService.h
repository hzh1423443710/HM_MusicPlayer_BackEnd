#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <mutex>

/**
 * @brief VerifyService 验证码服务 单例
 */
class VerifyService {
	struct VerifyInfo {
		std::string code;
		std::chrono::system_clock::time_point expire_time;
		std::string action;
	};

public:
	~VerifyService() = default;
	static VerifyService* getInstance();

	bool sendVerifyCode(const std::string& email, const std::string& action);

	bool verifyCode(const std::string& email, const std::string& action,
					const std::string& verify_code);

	// 清理过期的验证码
	size_t cleanupExpiredCodes();

private:
	VerifyService() = default;
	VerifyService(const VerifyService&) = delete;
	VerifyService(VerifyService&&) = delete;
	VerifyService& operator=(const VerifyService&) = delete;
	VerifyService& operator=(VerifyService&&) = delete;

private:
	std::map<std::string, VerifyInfo> m_codes;
	std::mutex m_mtx;
	static std::unique_ptr<VerifyService> m_instance;
};