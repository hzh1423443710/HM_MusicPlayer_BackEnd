#include "VerifyService.h"
#include "../utils/Config.h"
#include "../utils/EmailUtil.h"

#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>

std::unique_ptr<VerifyService> VerifyService::m_instance = nullptr;

VerifyService* VerifyService::getInstance() {
	if (m_instance) return m_instance.get();

	static std::once_flag flag;
	std::call_once(flag, []() { m_instance.reset(new VerifyService()); });

	return m_instance.get();
}

bool VerifyService::sendVerifyCode(const std::string& to_email, const std::string& action) {
	// 生成验证码
	std::string verify_code = EmailUtil::generateVerificationCode();

	bool success = EmailUtil::sendTextEmail(to_email, "Verification Code",
											"Your verification code is: " + verify_code);
	if (!success) return false; // 发送验证码失败

	std::chrono::seconds expiry_time(
		Config::getInstance()->getVerifyServiceConfig().verfication_code_expiry);

	// 存储验证码和过期时间
	std::lock_guard<std::mutex> lock(m_mtx);
	m_codes[to_email] = {verify_code, std::chrono::system_clock::now() + expiry_time, action};

	return true;
}

bool VerifyService::verifyCode(const std::string& email, const std::string& action,
							   const std::string& verify_code) {
	const VerifyInfo* pInfo{};
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		auto it = m_codes.find(email);
		if (it == m_codes.end()) {
			return false; // 验证码不存在
		}
		pInfo = &it->second;
	}

	// 验证码已过期或操作不匹配
	auto now = std::chrono::system_clock::now();
	return !(pInfo->expire_time < now || pInfo->action != action || pInfo->code != verify_code);
}

size_t VerifyService::cleanupExpiredCodes() {
	auto now = std::chrono::system_clock::now();

	std::lock_guard<std::mutex> lock(m_mtx);
	size_t count = 0;
	for (auto it = m_codes.begin(); it != m_codes.end();) {
		if (it->second.expire_time < now) {
			it = m_codes.erase(it);
			++count;
		} else {
			++it;
		}
	}

	return count;
}
