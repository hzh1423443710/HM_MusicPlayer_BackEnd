#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct DatabaseConfig {
	std::string host = "localhost";
	uint16_t port = 3306;
	std::string user = "root";
	std::string password;
	std::string dbname;
	size_t connection_pool_size = 5;
	uint32_t connection_timeout = 3; // 秒
};

struct ServerConfig {
	std::string host = "0.0.0.0";
	uint16_t port = 8080;
	int threads = 4;
	// bool enable_cors = true;
	// std::vector<std::string> cors_allowed_origins = {"*"};
	uint32_t request_timeout = 30; // 秒
};

struct JWTConfig {
	std::string secret;
	uint32_t expire_time = 86400; // 过期时间，默认1天
	std::string issuer = "music-player-backend";
};

struct LogConfig {
	std::string level = "info";		   // 日志级别
	std::string path = "logs/app.log"; // 日志文件路径
};

class Config {
public:
	~Config() = default;
	Config(const Config&) = default;
	Config(Config&&) = delete;
	Config& operator=(const Config&) = default;
	Config& operator=(Config&&) = delete;

	static Config* getInstance();

	bool loadFromFile(const std::string& filename);

	bool loadFromJson(const std::string& json_str);

	const DatabaseConfig& getDatabaseConfig() const { return m_db_config; }

	const ServerConfig& getServerConfig() const { return m_server_config; }

	const LogConfig& getLogConfig() const { return m_log_config; }

private:
	Config() = default;
	void parseDatabaseConfig(const nlohmann::json& j);
	void parseServerConfig(const nlohmann::json& j);
	void parseJWTConfig(const nlohmann::json& j);
	void parseLogConfig(const nlohmann::json& j);

private:
	DatabaseConfig m_db_config;
	ServerConfig m_server_config;
	JWTConfig m_jwt_config;
	LogConfig m_log_config;

	static std::unique_ptr<Config> m_instance;
};