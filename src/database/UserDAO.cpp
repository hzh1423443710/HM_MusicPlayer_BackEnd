#include "UserDAO.h"
#include "../utils/PasswordUtil.h"
#include "DBManager.h"
#include "jdbc/cppconn/datatype.h"
#include "jdbc/cppconn/exception.h"

#include <jdbc/cppconn/prepared_statement.h>
#include <spdlog/spdlog.h>
#include <optional>

constexpr const char* TAG = "[UserDAO]";

UserDAO::UserDAO() : db_manager{DBManager::getInstance()} {}

bool UserDAO::createUser(User& user) {
	try {
		std::string hashed_passwd = PasswordUtil::hashPassword(user.passwd_hash);

		SqlConnGuard guard(db_manager->getConnection());

		PreStmtPtr pstmt(guard->prepareStatement(
			"INSERT INTO users (username, passwd_hash, email) VALUES (?, ?, ?)"));
		pstmt->setString(1, user.username);
		pstmt->setString(2, hashed_passwd);
		pstmt->setString(3, user.email);
		pstmt->executeUpdate();

		StmtPtr stmt(guard->createStatement());
		ResultSetPtr result(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
		if (result->next()) {
			user.id = result->getInt("id");
			return true;
		}

		return false;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Create User Failed: {}, Code:{}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

std::optional<User> UserDAO::getUserById(int id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("SELECT * FROM users WHERE id = ?"));
		pstmt->setInt(1, id);

		ResultSetPtr result(pstmt->executeQuery());
		if (result->next()) {
			return buildFromResultSet(result);
		}

		return std::nullopt;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get User By ID Failed: {}, Code:{}", TAG, e.what(), e.getErrorCode());
		return std::nullopt;
	}
}

std::optional<User> UserDAO::getUserByUsernameOrEmail(const std::string& username_or_email) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(
			guard->prepareStatement("SELECT * FROM users WHERE username = ? OR email = ?"));
		pstmt->setString(1, username_or_email);
		pstmt->setString(2, username_or_email);

		ResultSetPtr result(pstmt->executeQuery());
		if (result->next()) {
			return buildFromResultSet(result);
		}

		return std::nullopt;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get User By Username or Email Failed: {}, Code:{}", TAG, e.what(),
					  e.getErrorCode());
		return std::nullopt;
	}
}

bool UserDAO::updateUser(const User& user) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement(
			"UPDATE users SET username = ?, email = ?, qq_id = ?, netease_id = ? WHERE id = ?"));

		pstmt->setString(1, user.username);
		pstmt->setString(2, user.email);

		if (!user.qq_id.empty())
			pstmt->setString(3, user.qq_id);
		else
			pstmt->setNull(3, sql::DataType::VARCHAR);

		if (!user.netease_id.empty())
			pstmt->setString(4, user.netease_id);
		else
			pstmt->setNull(4, sql::DataType::VARCHAR);

		pstmt->setInt(5, user.id);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (const sql::SQLException& e) {
		spdlog::error("{} Update User Failed: {}, Code: {}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

bool UserDAO::deleteUser(int id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("DELETE FROM users WHERE id = ?"));
		pstmt->setInt(1, id);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (const sql::SQLException& e) {
		spdlog::error("{} Delete User Failed: {}, Code:{}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

bool UserDAO::verifyPassword(const std::string& username_or_email, const std::string& password) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement(
			"SELECT passwd_hash FROM users WHERE username = ? OR email = ?"));
		pstmt->setString(1, username_or_email);
		pstmt->setString(2, username_or_email);
		ResultSetPtr result(pstmt->executeQuery());

		if (result->next()) {
			std::string hashed_passwd = result->getString("passwd_hash");
			return PasswordUtil::verifyPassword(password, hashed_passwd);
		}

		return false;
	} catch (const sql::SQLException& e) {
		spdlog::error("{} Verify Password Failed: {}, Code: {}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

bool UserDAO::updatePassword(int user_id, const std::string& new_password) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("UPDATE users SET passwd_hash = ? WHERE id = ?"));
		pstmt->setString(1, PasswordUtil::hashPassword(new_password));
		pstmt->setInt(2, user_id);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (const sql::SQLException& e) {
		spdlog::error("{} Update Password Failed: {}, Code:{}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

bool UserDAO::updateQQId(int user_id, const std::string& qq_id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("UPDATE users SET qq_id = ? WHERE id = ?"));
		pstmt->setString(1, qq_id);
		pstmt->setInt(2, user_id);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (const sql::SQLException& e) {
		spdlog::error("{} Update QQ ID Failed: {}, Code: {}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

bool UserDAO::updateNetEaseId(int user_id, const std::string& netease_id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("UPDATE users SET netease_id = ? WHERE id = ?"));
		pstmt->setString(1, netease_id);
		pstmt->setInt(2, user_id);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (const sql::SQLException& e) {
		spdlog::error("{} Update NetEase ID Failed: {}, Code: {}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

User UserDAO::buildFromResultSet(const ResultSetPtr& result) {
	User user;
	user.id = result->getInt("id");
	user.username = result->getString("username");
	user.passwd_hash = result->getString("passwd_hash");
	user.email = result->getString("email");
	user.create_at = result->getString("create_at");
	user.update_at = result->getString("update_at");

	if (!result->isNull("qq_id")) {
		user.qq_id = result->getString("qq_id");
	}
	if (!result->isNull("netease_id")) {
		user.netease_id = result->getString("netease_id");
	}

	return user;
}
