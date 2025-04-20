#pragma once

#include <string>
#include <vector>
#include "DBManager.h"
#include "../models/playhistory.h"
#include "../models/song.h"

/**
 * @brief 播放历史数据访问对象
 */
class PlayHistoryDAO {
public:
	PlayHistoryDAO();

	/**
	 * @brief 添加播放记录
	 * @param history 播放历史对象
	 * @return 是否添加成功
	 */
	bool addPlayHistory(const PlayHistory& history);

	/**
	 * @brief 获取用户的播放历史
	 * @param user_id 用户ID
	 * @param limit 限制返回数量
	 * @param offset 偏移量(分页用)
	 * @return 播放历史记录列表
	 */
	std::vector<PlayHistory> getUserPlayHistory(int user_id, int limit = 50, int offset = 0);

	/**
	 * @brief 清空用户播放历史
	 * @param user_id 用户ID
	 * @return 是否清空成功
	 */
	bool clearUserPlayHistory(int user_id);

	/**
	 * @brief 删除特定播放记录
	 * @param history_id 历史记录ID
	 * @param user_id 用户ID(用于验证权限)
	 * @return 是否删除成功
	 */
	bool deletePlayHistory(int history_id, int user_id);

	/**
	 * @brief 获取用户播放历史数目
	 * @param user_id 用户ID
	 * @return 播放总次数
	 */
	int getUserTotalPlayCount(int user_id);

	/**
	 * @brief 获取用户最近一周的播放次数
	 * @param user_id 用户ID
	 * @return 最近一周播放次数
	 */
	int getUserRecentPlayCount(int user_id, int days = 7);

	/**
	 * @brief 获取用户最常听的歌手
	 * @param user_id 用户ID
	 * @param limit 返回数量限制
	 * @return 歌手及播放次数列表
	 */
	std::vector<std::pair<std::string, int>> getUserTopArtists(int user_id, int limit = 5);

	/**
	 * @brief 获取用户最常听的歌曲
	 * @param user_id 用户ID
	 * @param limit 返回数量限制
	 * @return 歌曲列表
	 */
	std::vector<Song> getUserTopSongs(int user_id, int limit = 5);

	/**
	 * @brief 获取用户最常播放的歌曲（按播放次数排序）
	 * @param user_id 用户ID
	 * @param limit 返回数量限制
	 * @return 歌曲和播放次数列表
	 */
	std::vector<std::pair<PlayHistory, int>> getMostPlayedSongs(int user_id, int limit = 10);

private:
	DBManager* db_manager;

	/**
	 * @brief 从ResultSet构建PlayHistory对象
	 * @param result 结果集
	 * @return PlayHistory对象
	 */
	static PlayHistory buildFromResultSet(const ResultSetPtr& result);

	/**
	 * @brief 从ResultSet构建Song对象
	 * @param result 结果集
	 * @return Song对象
	 */
	static Song buildSongFromResultSet(const ResultSetPtr& result);
};