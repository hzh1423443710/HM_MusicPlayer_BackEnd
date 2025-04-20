#pragma once
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/resultset.h>

using SqlConnPtr = std::shared_ptr<sql::Connection>;
using PreStmtPtr = std::unique_ptr<sql::PreparedStatement>;
using ResultSetPtr = std::shared_ptr<sql::ResultSet>;

/**
 * @brief 数据库连接池管理类
 */
class DBManager {
public:
	static void init(std::string host, uint16_t port, std::string user, std::string passwd,
					 std::string db_name, size_t pool_size);
	static DBManager* getInstance();

	// 预处理 sql
	PreStmtPtr prepareStatement(const std::string& sql);
	// 执行 sql
	bool execute(const std::string& sql);
	// 执行查询 sql
	ResultSetPtr executeQuery(const std::string& sql);
	// 执行更新 sql
	int executeUpdate(const std::string& sql);

	~DBManager();
	DBManager(const DBManager&) = delete;
	DBManager(DBManager&&) = delete;
	DBManager& operator=(const DBManager&) = delete;
	DBManager& operator=(DBManager&&) = delete;

private:
	SqlConnPtr getConnection();

	DBManager(std::string host, uint16_t port, std::string user, std::string passwd,
			  std::string db_name, size_t pool_size);

private:
	sql::mysql::MySQL_Driver* m_driver;
	std::vector<SqlConnPtr> m_connections;
	std::mutex m_mtx;
	size_t m_next_conn{};

	std::string m_host;
	std::string m_user;
	std::string m_passwd;
	std::string m_db_name;
	uint16_t m_port;

	static std::unique_ptr<DBManager> m_instance;
};