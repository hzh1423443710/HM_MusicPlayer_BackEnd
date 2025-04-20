#include "PasswordUtil.h"

#include <array>
#include <iomanip>
#include <sstream>

#include <openssl/rand.h>
#include <openssl/evp.h>

std::string PasswordUtil::hashPassword(const std::string& password) {
	std::array<uint8_t, SHA256_DIGEST_LENGTH> hash{};
	unsigned int hashLen{};

	//  Message Digest
	EVP_MD_CTX* ctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
	EVP_DigestUpdate(ctx, password.data(), password.length());
	EVP_DigestFinal_ex(ctx, hash.data(), &hashLen);
	EVP_MD_CTX_free(ctx);

	return to_hex_string(hash);
}

bool PasswordUtil::verifyPassword(const std::string& password, const std::string& hashed_password) {
	return hashPassword(password) == hashed_password;
}

std::string PasswordUtil::to_hex_string(const std::array<uint8_t, SHA256_DIGEST_LENGTH>& data) {
	std::ostringstream oss;
	for (uint8_t i : data) {
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);
	}
	return oss.str();
}
