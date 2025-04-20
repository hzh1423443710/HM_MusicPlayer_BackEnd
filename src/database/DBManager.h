#pragma once
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>

#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/resultset.h>

using SqlConnPtr = std::shared_ptr<sql::Connection>;
using PreStmtPtr = std::unique_ptr<sql::PreparedStatement>;
using StmtPtr = std::unique_ptr<sql::Statement>;
using ResultSetPtr = std::shared_ptr<sql::ResultSet>;

/**
 * @brief 数据库连接池
 */
class DBManager {
public:
	static void init(std::string host, uint16_t port, std::string user, std::string passwd,
					 std::string db_name, size_t pool_size);
	static DBManager* getInstance();

	SqlConnPtr getConnection(std::chrono::seconds timeout = std::chrono::seconds(3));
	void releaseConnection(const SqlConnPtr& conn);
	size_t available() const { return m_connections.size(); }

	~DBManager();
	DBManager(const DBManager&) = delete;
	DBManager(DBManager&&) = delete;
	DBManager& operator=(const DBManager&) = delete;
	DBManager& operator=(DBManager&&) = delete;

private:
	DBManager(std::string host, uint16_t port, std::string user, std::string passwd,
			  std::string db_name, size_t pool_size);

private:
	sql::mysql::MySQL_Driver* m_driver;
	std::deque<SqlConnPtr> m_connections;
	mutable std::mutex m_mtx;
	std::condition_variable m_cv;

	std::string m_host;
	std::string m_user;
	std::string m_passwd;
	std::string m_db_name;
	uint16_t m_port;

	static std::unique_ptr<DBManager> m_instance;
};

/**
 * @brief 数据库连接 连接 RAII 封装
 */
struct SqlConnGuard {
	explicit SqlConnGuard(SqlConnPtr conn) : m_conn(std::move(conn)) {}
	~SqlConnGuard() {
		if (m_conn) {
			DBManager::getInstance()->releaseConnection(m_conn);
		}
	}

	SqlConnGuard(const SqlConnGuard&) = delete;
	SqlConnGuard(SqlConnGuard&&) = delete;
	SqlConnGuard& operator=(const SqlConnGuard&) = default;
	SqlConnGuard& operator=(SqlConnGuard&&) = default;

	// 获取原始连接
	SqlConnPtr get() const { return m_conn; }

	sql::Connection* operator->() const { return m_conn.get(); }

private:
	SqlConnPtr m_conn;
};