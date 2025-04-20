#pragma once
#include <string>
#include <cstdint>
#include <openssl/sha.h>

class PasswordUtil {
public:
	//  哈希密码
	static std::string hashPassword(const std::string& password);
	// 验证哈希
	static bool verifyPassword(const std::string& password, const std::string& hashed_password);

private:
	static std::string to_hex_string(const std::array<uint8_t, SHA256_DIGEST_LENGTH>& data);
};