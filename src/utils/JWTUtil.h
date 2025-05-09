#pragma once
#include <map>
#include <string>
#include <chrono>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <jwt-cpp/traits/kazuho-picojson/traits.h>

class JWTUtil {
public:
	JWTUtil(std::string secret) : m_secret(std::move(secret)) {}

	std::string generateToken(const std::string& subject,
							  const std::map<std::string, std::string>& claims = {},
							  const std::chrono::seconds& valid_for = std::chrono::hours{24}) const;

	std::string verifyToken(const std::string& token) const;

	static std::string getSubject(const std::string& token);

	static std::string getClaim(const std::string& token, const std::string& claim_name);
	static bool isTokenExpired(const std::string& token);

	void setIssuer(const std::string& issuer) noexcept { m_issuer = issuer; }

	std::string getIssuer() const noexcept { return m_issuer; }

private:
	std::string m_secret;
	std::string m_issuer = "auth0";
};
