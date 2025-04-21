#include "Config.h"

#include <spdlog/spdlog.h>

#include <fstream>
#include <mutex>

constexpr const char* TAG = "[Config]";

std::unique_ptr<Config> Config::m_instance = nullptr;

Config* Config::getInstance() {
	static std::once_flag flag;
	std::call_once(flag, []() { m_instance.reset(new Config()); });

	return m_instance.get();
}

bool Config::loadFromFile(const std::string& filename) {
	std::ifstream ifs(filename);
	if (!ifs) {
		spdlog::error("{} Failed to open config file: {}", TAG, filename);
		return false;
	}

	try {
		json config_json;
		ifs >> config_json;

		if (config_json.contains("database")) {
			parseDatabaseConfig(config_json["database"]);
		}

		if (config_json.contains("server")) {
			parseServerConfig(config_json["server"]);
		}

		if (config_json.contains("jwt")) {
			parseJWTConfig(config_json["jwt"]);
		}

		if (config_json.contains("log")) {
			parseLogConfig(config_json["log"]);
		}

		spdlog::info("{} Config loaded successfully", TAG);

		return true;
	} catch (const json::parse_error& e) {
		spdlog::error("{} JSON parse error: {}", TAG, e.what());
		return false;
	} catch (const std::exception& e) {
		spdlog::error("{} Failed to parse config file: {}", TAG, e.what());
		return false;
	}
}

void Config::parseDatabaseConfig(const nlohmann::json& j) {
	if (j.contains("host") && j["host"].is_string())
		m_db_config.host = j["host"].get<std::string>();
	else
		throw std::runtime_error("Database host is required");

	if (j.contains("port") && j["port"].is_number_integer())
		m_db_config.port = j["port"].get<uint16_t>();
	else
		throw std::runtime_error("Database port is required");

	if (j.contains("user") && j["user"].is_string())
		m_db_config.user = j["user"].get<std::string>();
	else
		throw std::runtime_error("Database user is required");

	if (j.contains("password") && j["password"].is_string())
		m_db_config.password = j["password"].get<std::string>();
	else
		throw std::runtime_error("Database password is required");

	if (j.contains("dbname") && j["dbname"].is_string())
		m_db_config.dbname = j["dbname"].get<std::string>();
	else
		throw std::runtime_error("Database name is required");

	if (j.contains("connection_pool_size") && j["connection_pool_size"].is_number_integer())
		m_db_config.connection_pool_size = j["connection_pool_size"].get<size_t>();
	else
		throw std::runtime_error("Database connection pool size is required");

	if (j.contains("connection_timeout") && j["connection_timeout"].is_number_integer())
		m_db_config.connection_timeout = j["connection_timeout"].get<uint32_t>();
	else
		throw std::runtime_error("Database connection timeout is required");
}

void Config::parseServerConfig(const nlohmann::json& j) {
	if (j.contains("host") && j["host"].is_string())
		m_server_config.host = j["host"].get<std::string>();
	else
		throw std::runtime_error("Server host is required");

	if (j.contains("port") && j["port"].is_number_integer())
		m_server_config.port = j["port"].get<uint16_t>();
	else
		throw std::runtime_error("Server port is required");

	if (j.contains("threads") && j["threads"].is_number_integer())
		m_server_config.threads = j["threads"].get<int>();
	else
		throw std::runtime_error("Server threads is required");

	if (j.contains("request_timeout") && j["request_timeout"].is_number_integer())
		m_server_config.request_timeout = j["request_timeout"].get<uint32_t>();
	else
		throw std::runtime_error("Server request timeout is required");
}

void Config::parseJWTConfig(const nlohmann::json& j) {
	if (j.contains("secret") && j["secret"].is_string())
		m_jwt_config.secret = j["secret"].get<std::string>();
	else
		throw std::runtime_error("JWT secret is required");

	if (j.contains("expire_time") && j["expire_time"].is_number_integer())
		m_jwt_config.expire_time = j["expire_time"].get<uint32_t>();
	else
		throw std::runtime_error("JWT expire time is required");

	if (j.contains("issuer") && j["issuer"].is_string())
		m_jwt_config.issuer = j["issuer"].get<std::string>();
	else
		throw std::runtime_error("JWT issuer is required");
}

void Config::parseLogConfig(const nlohmann::json& j) {
	if (j.contains("level") && j["level"].is_string())
		m_log_config.level = j["level"].get<std::string>();
	else
		throw std::runtime_error("Log level is required");

	if (j.contains("path") && j["path"].is_string())
		m_log_config.path = j["path"].get<std::string>();
	else
		throw std::runtime_error("Log path is required");
}
