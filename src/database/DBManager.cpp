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
	spdlog::info("{} Close connection pool success", TAG);
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

SqlConnPtr DBManager::getConnection() {
	std::lock_guard<std::mutex> lock(m_mtx);

	if (m_connections.empty()) {
		throw std::runtime_error("Database connection pool is empty");
	}

	// 轮询分配连接
	SqlConnPtr conn = m_connections[m_next_conn];
	m_next_conn = (m_next_conn + 1) % m_connections.size();

	try {
		// 无效重新连接
		if (!conn->isValid()) {
			spdlog::warn("{} Connection is not valid, try to reconnect", TAG);
			conn->close();
			conn.reset(m_driver->connect("tcp://" + m_host + ":" + std::to_string(m_port), m_user,
										 m_passwd));
			conn->setSchema(m_db_name);
		}

	} catch (sql::SQLException& e) {
		spdlog::error("{} Reconnect failed: {}, Error Code:{}, SQLState:{}", TAG, e.what(),
					  e.getErrorCode(), e.getSQLStateCStr());
	}

	return conn;
}

PreStmtPtr DBManager::prepareStatement(const std::string& sql) {
	SqlConnPtr conn = getConnection();

	try {
		return PreStmtPtr(conn->prepareStatement(sql));

	} catch (sql::SQLException& e) {
		spdlog::error("{} Prepare SQL failed: {}, Error Code:{}, SQLState:{}", TAG, e.what(),
					  e.getErrorCode(), e.getSQLStateCStr());
		throw;
	}
}

bool DBManager::execute(const std::string& sql) {
	SqlConnPtr conn = getConnection();

	try {
		std::unique_ptr<sql::Statement> stmt(conn->createStatement());
		return stmt->execute(sql);

	} catch (sql::SQLException& e) {
		spdlog::error("{} Execute SQL failed: {}, Error Code:{}, SQLState:{}", TAG, e.what(),
					  e.getErrorCode(), e.getSQLStateCStr());
		return false;
	}
}

ResultSetPtr DBManager::executeQuery(const std::string& sql) {
	SqlConnPtr conn = getConnection();
	try {
		std::unique_ptr<sql::Statement> stmt(conn->createStatement());
		return ResultSetPtr(stmt->executeQuery(sql));

	} catch (sql::SQLException& e) {
		spdlog::error("{} Execute SQL Query failed: {}, Error Code:{}, SQLState:{}", TAG, e.what(),
					  e.getErrorCode(), e.getSQLStateCStr());
		return nullptr;
	}
}

int DBManager::executeUpdate(const std::string& sql) {
	SqlConnPtr conn = getConnection();
	try {
		std::unique_ptr<sql::Statement> stmt(conn->createStatement());
		return stmt->executeUpdate(sql);

	} catch (sql::SQLException& e) {
		spdlog::error("{} Execute SQL Update failed: {}, Error Code:{}, SQLState:{}", TAG, e.what(),
					  e.getErrorCode(), e.getSQLStateCStr());
		return -1;
	}
}
