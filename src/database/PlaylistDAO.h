#pragma once

#include <optional>
#include <string>
#include <vector>
#include "DBManager.h"
#include "../models/playlist.h"
#include "../models/song.h"

/**
 * @brief 歌单数据访问对象
 */
class PlaylistDAO {
public:
	PlaylistDAO();

	// 歌单 CRUD
	bool createPlaylist(Playlist& playlist);
	std::optional<Playlist> getPlaylistById(int playlist_id);
	std::vector<Playlist> getPlaylistsByUserId(int user_id);
	bool updatePlaylist(const Playlist& playlist);
	bool deletePlaylist(int id);

	// 歌单歌曲 CRUD
	bool addSongToPlaylist(int playlist_id, const Song& song);
	bool removeSongFromPlaylist(int playlist_id, const std::string& song_id,
								std::string& song_source);
	std::vector<Song> getSongsInPlaylist(int playlist_id);

	int countSongsInPlaylist(int playlist_id);

private:
	DBManager* db_manager;

	static Playlist buildPlaylistFromResultSet(const ResultSetPtr& result);
	static Song buildSongFromResultSet(const ResultSetPtr& result);
};