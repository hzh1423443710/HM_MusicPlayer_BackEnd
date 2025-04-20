#include "JWTUtil.h"

// 生成 JWT Token
std::string JWTUtil::generateToken(const std::string& subject,
								   const std::map<std::string, std::string>& claims,
								   const std::chrono::seconds& valid_for) const {
	try {
		auto now = std::chrono::system_clock::now();
		auto token = jwt::create()
						 .set_issuer(m_issuer)
						 .set_type("JWT")
						 .set_issued_at(now)
						 .set_expires_at(now + valid_for)
						 .set_subject(subject);

		// payload claims
		for (const auto& claim : claims) {
			token.set_payload_claim(claim.first, jwt::claim(claim.second));
		}

		// HS256 算法签名
		return token.sign(jwt::algorithm::hs256{m_secret});
	} catch (const std::exception& e) {
		throw std::runtime_error("Error generating token: " + std::string(e.what()));
	}
}

// 验证 JWT Token 返回解码后的 JWT
jwt::decoded_jwt<jwt::traits::kazuho_picojson> JWTUtil::verifyToken(
	const std::string& token) const {
	try {
		auto decoded = jwt::decode(token);

		// 验证器
		auto verifer = jwt::verify()
						   .allow_algorithm(jwt::algorithm::hs256{m_secret})
						   .with_issuer(m_issuer)
						   .leeway(60); // 允许60s时间偏差

		verifer.verify(decoded);
		return decoded;
	} catch (const std::exception& e) {
		throw std::runtime_error("Error verifying token: " + std::string(e.what()));
	}
}

std::string JWTUtil::getSubject(const std::string& token) {
	try {
		auto decoded = jwt::decode(token);
		return decoded.get_subject();
	} catch (const std::exception& e) {
		throw std::runtime_error("Error getting subject: " + std::string(e.what()));
	}
}

std::string JWTUtil::getClaim(const std::string& token, const std::string& claim_name) {
	try {
		auto decoded = jwt::decode(token);
		return decoded.get_payload_claim(claim_name).as_string();
	} catch (const std::exception& e) {
		throw std::runtime_error("Error getting claim: " + std::string(e.what()));
	}
}

bool JWTUtil::isTokenExpired(const std::string& token) {
	try {
		auto decoded = jwt::decode(token);
		auto exp = decoded.get_expires_at();
		auto now = std::chrono::system_clock::now();
		return exp < now;
	} catch (const std::exception& e) {
		throw std::runtime_error("Error checking token expiration: " + std::string(e.what()));
	}
}
