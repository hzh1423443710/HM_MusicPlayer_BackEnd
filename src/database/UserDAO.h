#pragma once
#include <optional>
#include <string>

#include "../models/user.h"
#include "DBManager.h"

/**
 * @brief 用户数据访问对象
 */
class UserDAO {
public:
	UserDAO();

	/**
	 * @brief 创建用户(username, email, passwd)
	 */
	bool createUser(User& user);
	std::optional<User> getUserById(int id);
	std::optional<User> getUserByUsernameOrEmail(const std::string& username_or_email);

	/**
	 * @brief 更新 username, email, qq_id, netease_id
	 */
	bool updateUser(const User& user);
	bool deleteUser(int id);

	// 用户登录/修改密码
	bool verifyPassword(const std::string& username_or_email, const std::string& password);
	bool updatePassword(int user_id, const std::string& new_password);

	// 关联QQ/NetEase
	bool updateQQId(int user_id, const std::string& qq_id);
	bool updateNetEaseId(int user_id, const std::string& netease_id);

private:
	DBManager* db_manager;

	static User buildFromResultSet(const ResultSetPtr& result);
};