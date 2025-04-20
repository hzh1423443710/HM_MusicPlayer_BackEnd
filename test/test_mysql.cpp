#include <iostream>

#include <database/UserDAO.h>
#include <database/PlaylistDAO.h>
#include "database/DBManager.h"
#include "models/user.h"

#include "utils/PasswordUtil.h"

void test_user_dao() {
	UserDAO user_dao;
	User user{.id = 3,
			  .username = "hzh",
			  .passwd_hash = "123456",
			  .email = "142344@qq.com",
			  .qq_id = "1423443710"};

	if (user_dao.createUser(user)) {
		std::cout << "User created successfully!\n";
	} else {
		std::cout << "Failed to create user!\n";
	}

	auto ret_user = user_dao.getUserByUsername("hzh");
	if (ret_user) {
		std::cout << "User found: " << ret_user->username << "\n";
	} else {
		std::cout << "User not found!\n";
	}

	user_dao.updatePassword(ret_user->id, "12345678");

	if (user_dao.verifyPassword("hzh", "12345678")) {
		std::cout << "Password verified successfully!\n";
	} else {
		std::cout << "Password verification failed!\n";
	}
}

int main() {
	DBManager::init("119.3.185.203", 4406, "root", "123456", "HW_MusicPlayer", 5);
	test_user_dao();

	return 0;
}
