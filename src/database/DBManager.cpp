#include "DBManager.h"

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_error.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <mutex>
#include <string>
#include <utility>

constexpr const char* TAG = "[DBManager]";

std::unique_ptr<DBManager> DBManager::m_instance = nullptr;

DBManager::~DBManager() {
	for (auto& conn : m_connections) {
		try {
			conn->close();
		} catch (sql::SQLException& e) {
			spdlog::error("{} Close connection failed: {}, Error Code:{}, SQLState:{}", TAG,
						  e.what(), e.getErrorCode(), e.getSQLStateCStr());
		}
	}

	m_connections.clear();
	// TODO exception ?
	// spdlog::info("{} Close connection pool success", TAG);
}

DBManager::DBManager(std::string host, uint16_t port, std::string user, std::string passwd,
					 std::string db_name, size_t pool_size)
	: m_driver(sql::mysql::get_mysql_driver_instance()),
	  m_host{std::move(host)},
	  m_user{std::move(user)},
	  m_passwd{std::move(passwd)},
	  m_db_name{std::move(db_name)},
	  m_port{port} {
	const std::string& host_name = "tcp://" + m_host + ":" + std::to_string(m_port);

	try {
		// 创建 数据库 连接池
		for (size_t i = 0; i < pool_size; ++i) {
			auto* conn = m_driver->connect(host_name, m_user, m_passwd);
			conn->setSchema(m_db_name);
			this->m_connections.push_back(SqlConnPtr{conn});
		}

		spdlog::info("{} Create connection pool success, size: {}", TAG, pool_size);
	} catch (sql::SQLException& e) {
		spdlog::critical(
			"{} Create connection pool failed: {}, HostName:{}, Error Code:{}, SQLState:{}", TAG,
			e.what(), host_name, e.getErrorCode(), e.getSQLStateCStr());
	}
}

void DBManager::init(std::string host, uint16_t port, std::string user, std::string passwd,
					 std::string db_name, size_t pool_size) {
	if (!m_instance) {
		m_instance.reset(new DBManager(std::move(host), port, std::move(user), std::move(passwd),
									   std::move(db_name), pool_size));
	}
}

DBManager* DBManager::getInstance() {
	if (!m_instance) {
		throw std::runtime_error("DBManager not initialized");
	}

	return m_instance.get();
}

SqlConnPtr DBManager::getConnection(std::chrono::seconds timeout) {
	SqlConnPtr conn;
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		if (m_connections.empty()) {
			m_cv.wait_for(lock, timeout, [this]() { return !m_connections.empty(); });
		}

		if (m_connections.empty()) {
			spdlog::error("{} No available connection", TAG);
			return nullptr;
		}

		conn = m_connections.front();
		m_connections.pop_front();
	}

	try {
		// 无效重新连接
		if (!conn->isValid()) {
			spdlog::warn("{} Connection is not valid, try to reconnect", TAG);
			conn->reconnect();
			// conn->close();
			// conn.reset(m_driver->connect("tcp://" + m_host + ":" + std::to_string(m_port),
			// m_user, m_passwd));
			conn->setSchema(m_db_name);
		}

	} catch (sql::SQLException& e) {
		spdlog::error("{} Reconnect failed: {}, Error Code:{}, SQLState:{}", TAG, e.what(),
					  e.getErrorCode(), e.getSQLStateCStr());
	}

	return conn;
}

void DBManager::releaseConnection(const SqlConnPtr& conn) {
	std::lock_guard<std::mutex> lock(m_mtx);
	m_connections.push_back(conn);
	m_cv.notify_one();
}
