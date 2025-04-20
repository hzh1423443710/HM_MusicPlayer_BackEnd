#include "PlayHistoryDAO.h"
#include <spdlog/spdlog.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <vector>
#include "DBManager.h"

constexpr const char* TAG = "[PlayHistoryDAO]";

PlayHistoryDAO::PlayHistoryDAO() : db_manager{DBManager::getInstance()} {}

bool PlayHistoryDAO::addPlayHistory(const PlayHistory& history) {
	try {
		// 查询是否 已存在
		PreStmtPtr check_pstmt = db_manager->prepareStatement(
			"SELECT id, song_count FROM play_history WHERE user_id = ? AND song_id = ?");
		check_pstmt->setInt(1, history.user_id);
		check_pstmt->setString(2, history.song_id);

		ResultSetPtr check_result(check_pstmt->executeQuery());
		if (check_result->next()) {
			int song_count = check_result->getInt("song_count");
			int id = check_result->getInt("id");

			// 更新播放次数
			PreStmtPtr update_pstmt =
				db_manager->prepareStatement("UPDATE play_history SET song_count = ? WHERE id = ?");
			update_pstmt->setInt(1, song_count + 1);
			update_pstmt->setInt(2, id);

			return update_pstmt->executeUpdate() > 0;
		}

		// 插入新的播放记录
		PreStmtPtr insert_pstmt = db_manager->prepareStatement(
			"INSERT INTO play_history (user_id, song_id, song_name, song_singer, song_pic, "
			"song_where, played_at) VALUES (?, ?, ?, ?, ?, ?)");
		insert_pstmt->setInt(1, history.user_id);
		insert_pstmt->setString(2, history.song_id);
		insert_pstmt->setString(3, history.song_name);
		insert_pstmt->setString(4, history.song_singer);
		if (!history.song_pic.empty())
			insert_pstmt->setString(5, history.song_pic);
		else
			insert_pstmt->setNull(5, sql::DataType::VARCHAR);
		insert_pstmt->setString(6, history.song_where);

		return insert_pstmt->executeUpdate() > 0;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Add play history failed: {}, Code: {}", TAG, e.what(), e.getErrorCode());
		return false;
	}
}

std::vector<PlayHistory> PlayHistoryDAO::getUserPlayHistory(int user_id, int limit, int offset) {
	std::vector<PlayHistory> history_list;
	try {
		PreStmtPtr pstmt = db_manager->prepareStatement(
			"SELECT * FROM play_history WHERE user_id = ? ORDER BY played_at DESC LIMIT ? OFFSET "
			"?");

		pstmt->setInt(1, user_id);
		pstmt->setInt(2, limit);
		pstmt->setInt(3, offset);

		ResultSetPtr result(pstmt->executeQuery());
		history_list.reserve(limit);

		while (result->next()) {
			history_list.emplace_back(buildFromResultSet(result));
		}

		return history_list;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get user play history failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return {};
	}
}

bool PlayHistoryDAO::clearUserPlayHistory(int user_id) {
	try {
		PreStmtPtr pstmt =
			db_manager->prepareStatement("DELETE FROM play_history WHERE user_id = ?");
		pstmt->setInt(1, user_id);
		int affected_rows = pstmt->executeUpdate();

		return affected_rows > 0;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Clear user play history failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return false;
	}
}

bool PlayHistoryDAO::deletePlayHistory(int history_id, int user_id) {
	try {
		PreStmtPtr pstmt =
			db_manager->prepareStatement("DELETE FROM play_history WHERE id = ? AND user_id = ?");
		pstmt->setInt(1, history_id);
		pstmt->setInt(2, user_id);
		int affected_rows = pstmt->executeUpdate();

		return affected_rows > 0;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Delete play history failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return false;
	}
}

int PlayHistoryDAO::getUserTotalPlayCount(int user_id) {
	try {
		PreStmtPtr pstmt = db_manager->prepareStatement(
			"SELECT COUNT(*) AS count FROM play_history WHERE user_id = ?");

		pstmt->setInt(1, user_id);
		ResultSetPtr result(pstmt->executeQuery());

		if (result->next()) {
			return result->getInt("count");
		}

		return 0;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get user total play count failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return 0;
	}
}

int PlayHistoryDAO::getUserRecentPlayCount(int user_id, int days) {
	try {
		PreStmtPtr pstmt = db_manager->prepareStatement(
			"SELECT COUNT(*) AS count FROM play_history WHERE user_id = ? AND played_at >= NOW() - "
			"INTERVAL ? DAY");

		pstmt->setInt(1, user_id);
		pstmt->setInt(2, days);
		ResultSetPtr result(pstmt->executeQuery());

		if (result->next()) {
			return result->getInt("count");
		}

		return 0;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get user recent play count failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return 0;
	}
}

std::vector<std::pair<std::string, int>> PlayHistoryDAO::getUserTopArtists(int user_id, int limit) {
	std::vector<std::pair<std::string, int>> artists;

	try {
		PreStmtPtr pstmt = db_manager->prepareStatement(
			"SELECT song_singer, SUM(song_count) AS total_plays FROM play_history WHERE user_id = "
			"? GROUP BY song_singer ORDER BY total_plays DESC LIMIT ?");

		pstmt->setInt(1, user_id);
		pstmt->setInt(2, limit);
		ResultSetPtr result(pstmt->executeQuery());

		while (result->next()) {
			std::string artist = result->getString("song_singer");
			int play_count = result->getInt("total_plays");
			artists.emplace_back(artist, play_count);
		}

		return artists;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get user top artists failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return {};
	}
}

std::vector<Song> PlayHistoryDAO::getUserTopSongs(int user_id, int limit) {
	std::vector<Song> songs;

	try {
		PreStmtPtr pstmt = db_manager->prepareStatement(
			"SELECT * FROM play_history WHERE user_id = ? ORDER BY song_count DESC LIMIT ?");

		pstmt->setInt(1, user_id);
		pstmt->setInt(2, limit);
		ResultSetPtr result(pstmt->executeQuery());

		while (result->next()) {
			songs.emplace_back(buildSongFromResultSet(result));
		}

		return songs;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get user top songs failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return {};
	}
}

std::vector<std::pair<PlayHistory, int>> PlayHistoryDAO::getMostPlayedSongs(int user_id,
																			int limit) {
	std::vector<std::pair<PlayHistory, int>> songs;

	try {
		PreStmtPtr pstmt = db_manager->prepareStatement(
			"SELECT * FROM play_history WHERE user_id = ? ORDER BY song_count DESC LIMIT ?");
		pstmt->setInt(1, user_id);
		pstmt->setInt(2, limit);

		ResultSetPtr result(pstmt->executeQuery());
		while (result->next()) {
			PlayHistory history = buildFromResultSet(result);
			int play_count = result->getInt("song_count");
			songs.emplace_back(history, play_count);
		}

		return songs;
	} catch (sql::SQLException& e) {
		spdlog::error("{} Get most played songs failed: {}, Code: {}", TAG, e.what(),
					  e.getErrorCode());
		return {};
	}
}

PlayHistory PlayHistoryDAO::buildFromResultSet(const ResultSetPtr& result) {
	PlayHistory history;
	history.id = result->getInt("id");
	history.user_id = result->getInt("user_id");
	history.song_id = result->getString("song_id");
	history.song_name = result->getString("song_name");
	history.song_singer = result->getString("song_singer");
	if (!result->isNull("song_pic")) history.song_pic = result->getString("song_pic");
	history.song_where = result->getString("song_where");
	history.song_count = result->getInt("song_count");
	history.played_at = result->getString("played_at");

	return history;
}

Song PlayHistoryDAO::buildSongFromResultSet(const ResultSetPtr& result) {
	Song song;
	song.song_id = result->getString("song_id");
	song.name = result->getString("song_name");
	song.singer = result->getString("song_singer");
	if (!result->isNull("song_pic")) song.pic = result->getString("song_pic");
	song.where = result->getString("song_where");

	return song;
}
