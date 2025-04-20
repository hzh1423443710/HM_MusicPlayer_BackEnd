#include "PlaylistDAO.h"
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <spdlog/spdlog.h>
#include "DBManager.h"

constexpr const char* TAG = "[PlaylistDAO]";

PlaylistDAO::PlaylistDAO() : db_manager{DBManager::getInstance()} {}

bool PlaylistDAO::createPlaylist(Playlist& playlist) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement(
			"INSERT INTO playlists (name, user_id, cover) VALUES (?, ?, ?)"));
		pstmt->setString(1, playlist.name);
		pstmt->setInt(2, playlist.user_id);
		if (playlist.cover.empty())
			pstmt->setNull(3, sql::DataType::VARCHAR);
		else
			pstmt->setString(3, playlist.cover);

		pstmt->executeUpdate();

		ResultSetPtr result(pstmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
		if (result->next()) {
			playlist.id = result->getInt("id");
			return true;
		}

		return false;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Create playlist failed: {}, Code: {}", e.what(), e.getErrorCode());
		return false;
	}
}

std::optional<Playlist> PlaylistDAO::getPlaylistById(int playlist_id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("SELECT * FROM playlists WHERE id = ?"));
		pstmt->setInt(1, playlist_id);

		ResultSetPtr result(pstmt->executeQuery());
		if (result->next()) {
			return buildPlaylistFromResultSet(result);
		}

		return std::nullopt;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Get playlist by id failed: {}, Code: {}", e.what(), e.getErrorCode());
		return std::nullopt;
	}
}

std::vector<Playlist> PlaylistDAO::getPlaylistsByUserId(int user_id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("SELECT * FROM playlists WHERE user_id = ?"));
		pstmt->setInt(1, user_id);

		ResultSetPtr result(pstmt->executeQuery());
		std::vector<Playlist> playlists;
		playlists.reserve(result->rowsCount());

		while (result->next()) {
			playlists.emplace_back(buildPlaylistFromResultSet(result));
		}

		return playlists;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Get playlists by user id failed: {}, Code: {}", e.what(),
					  e.getErrorCode());
		return {};
	}
}

bool PlaylistDAO::updatePlaylist(const Playlist& playlist) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(
			guard->prepareStatement("UPDATE playlists SET name = ?, cover = ? WHERE id = ?"));
		pstmt->setString(1, playlist.name);

		if (playlist.cover.empty())
			pstmt->setNull(2, sql::DataType::VARCHAR);
		else
			pstmt->setString(2, playlist.cover);

		pstmt->setInt(3, playlist.id);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Update playlist failed: {}, Code: {}", e.what(), e.getErrorCode());
		return false;
	}
}

bool PlaylistDAO::deletePlaylist(int id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement("DELETE FROM playlists WHERE id = ?"));
		pstmt->setInt(1, id);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Delete playlist failed: {}, Code: {}", e.what(), e.getErrorCode());
		return false;
	}
}

bool PlaylistDAO::addSongToPlaylist(int playlist_id, const Song& song) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		// 是否已存在
		PreStmtPtr pstmt(guard->prepareStatement(
			"SELECT COUNT(*) as count FROM playlist_songs WHERE playlist_id = ? AND song_id = ? "
			"AND song_where = ?"));
		pstmt->setInt(1, playlist_id);
		pstmt->setString(2, song.song_id);
		pstmt->setString(3, song.where);
		ResultSetPtr result(pstmt->executeQuery());

		if (result->next() && result->getInt("count") > 0) {
			return true;
		}

		// 插入
		pstmt.reset(guard->prepareStatement(
			"INSERT INTO playlist_songs (playlist_id, song_id, song_name, song_singer, song_pic, "
			"song_where) VALUES (?, ?, ?, ?, ?, ?)"));
		pstmt->setInt(1, playlist_id);
		pstmt->setString(2, song.song_id);
		pstmt->setString(3, song.name);
		pstmt->setString(4, song.singer);
		if (song.pic.empty())
			pstmt->setNull(5, sql::DataType::VARCHAR);
		else
			pstmt->setString(5, song.pic);
		pstmt->setString(6, song.where);

		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Add song to playlist failed: {}, Code: {}", e.what(), e.getErrorCode());
		return false;
	}
}

bool PlaylistDAO::removeSongFromPlaylist(int playlist_id, const std::string& song_id,
										 std::string& song_source) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(
			guard->prepareStatement("DELETE FROM playlist_songs WHERE playlist_id = ? "
									"AND song_id = ? && song_where = ?"));

		pstmt->setInt(1, playlist_id);
		pstmt->setString(2, song_id);
		pstmt->setString(3, song_source);
		int affected_row = pstmt->executeUpdate();

		return affected_row > 0;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Remove song from playlist failed: {}, Code: {}", e.what(),
					  e.getErrorCode());
		return false;
	}
}

std::vector<Song> PlaylistDAO::getSongsInPlaylist(int playlist_id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(
			guard->prepareStatement("SELECT * FROM playlist_songs WHERE playlist_id = ?"));
		pstmt->setInt(1, playlist_id);
		ResultSetPtr result(pstmt->executeQuery());

		std::vector<Song> songs;
		songs.reserve(result->rowsCount());
		while (result->next()) {
			songs.emplace_back(buildSongFromResultSet(result));
		}

		return songs;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Get songs in playlist failed: {}, Code: {}", e.what(),
					  e.getErrorCode());
		return {};
	}
}

int PlaylistDAO::countSongsInPlaylist(int playlist_id) {
	try {
		SqlConnGuard guard(db_manager->getConnection());
		PreStmtPtr pstmt(guard->prepareStatement(
			"SELECT COUNT(*) as count FROM playlist_songs WHERE playlist_id = ?"));

		pstmt->setInt(1, playlist_id);
		ResultSetPtr result(pstmt->executeQuery());
		if (result->next()) {
			return result->getInt("count");
		}

		return 0;
	} catch (sql::SQLException& e) {
		spdlog::error(TAG, "Count songs in playlist failed: {}, Code: {}", e.what(),
					  e.getErrorCode());
		return 0;
	}
}

Playlist PlaylistDAO::buildPlaylistFromResultSet(const ResultSetPtr& result) {
	Playlist playlist;
	playlist.id = result->getInt("id");
	playlist.name = result->getString("name");
	playlist.user_id = result->getInt("user_id");

	playlist.cover = result->isNull("cover") ? "" : result->getString("cover");

	playlist.create_at = result->getString("create_at");
	playlist.update_at = result->getString("update_at");

	return playlist;
}

Song PlaylistDAO::buildSongFromResultSet(const ResultSetPtr& result) {
	Song song;
	song.id = result->getInt("id");
	song.song_id = result->getString("song_id");
	song.name = result->getString("song_name");
	song.singer = result->getString("song_singer");
	song.where = result->getString("song_where");
	song.pic = result->isNull("song_pic") ? "" : result->getString("song_pic");
	song.added_at = result->getString("added_at");

	return song;
}
