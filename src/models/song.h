#pragma once

#include <string>

/**
 * @brief 歌曲
 */
struct Song {
	int id;
	std::string song_id;
	std::string name;
	std::string singer;
	std::string pic;
	std::string where;
	std::string added_at;
};