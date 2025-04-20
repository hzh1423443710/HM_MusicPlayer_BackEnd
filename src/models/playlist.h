#pragma once
#include <string>

/**
 * @brief 歌单
 */
struct Playlist {
	int id = 0;
	int user_id = 0;
	std::string name;
	std::string cover;
	std::string create_at;
	std::string update_at;
};