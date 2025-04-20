#pragma once
#include <string>

struct User {
	int id = 0;
	std::string username;
	std::string passwd_hash;
	std::string email;
	std::string qq_id;
	std::string netease_id;
	std::string create_at;
	std::string update_at;
};