#include <iostream>
#include <string>

#include <database/UserDAO.h>
#include <database/PlaylistDAO.h>
#include "database/DBManager.h"
#include "models/user.h"

#include "utils/PasswordUtil.h"
#include "utils/EmailUtil.h"

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

void test_send_email(const std::string& to) {
	EmailUtil email_util("smtp.126.com", 587, "h1423443710@126.com", "RKjDMqAnaAQfEa7k");
	std::string verify_code = EmailUtil::generateVerificationCode();
	email_util.sendTextEmail(to, "Verification Code", "Your verification code is: " + verify_code);
}

int main(int argc, char* argv[]) {
	// DBManager::init("119.3.185.203", 4406, "root", "123456", "HW_MusicPlayer", 5);
	// test_user_dao();
	test_send_email(argv[1]);

	return 0;
}
