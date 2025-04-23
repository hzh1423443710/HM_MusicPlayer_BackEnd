#pragma once
#include <string>

/**
 * @brief 邮件发送工具类(SMTP)
 */
class EmailUtil {
	struct EmailData {
		const char* data;
		size_t size_left;
	};

public:
	// 发送文本邮件
	static bool sendTextEmail(const std::string& to, const std::string& subject,
							  const std::string& body);

	static std::string generateVerificationCode(int length = 6);

private:
	static size_t readCallback(void* ptr, size_t size, size_t nmemb, void* data);
};