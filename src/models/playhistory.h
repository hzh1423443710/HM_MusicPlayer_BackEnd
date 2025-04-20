#pragma once

#include <string>

/**
 * @brief 播放历史
 */
struct PlayHistory {
	int id = 0;
	int user_id = 0;
	std::string song_id;
	std::string song_name;
	std::string song_singer;
	std::string song_pic;
	std::string song_where;
	std::string played_at;
	int song_count = 0;
};