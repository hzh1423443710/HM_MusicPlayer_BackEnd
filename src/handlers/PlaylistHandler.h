// PlaylistHandler.h
#pragma once
#include "../database/PlaylistDAO.h"
#include "../utils/JWTUtil.h"
#include "../common/net.h"

class PlaylistHandler {
public:
	PlaylistHandler(const std::string& jwt_secret);

	// 创建歌单
	HttpResponse handleCreatePlaylist(const HttpRequest& req);

	// 获取用户的所有歌单
	HttpResponse handleGetPlaylists(const HttpRequest& req);

	// 获取单个歌单详情
	HttpResponse handleGetPlaylistById(const HttpRequest& req);

	// 更新歌单信息
	HttpResponse handleUpdatePlaylist(const HttpRequest& req);

	// 删除歌单
	HttpResponse handleDeletePlaylist(const HttpRequest& req);

	// 歌单中添加歌曲
	HttpResponse handleAddSongToPlaylist(const HttpRequest& req);

	// 从歌单中删除歌曲
	HttpResponse handleRemoveSongFromPlaylist(const HttpRequest& req);

	// 获取歌单中的所有歌曲
	HttpResponse handleGetSongsInPlaylist(const HttpRequest& req);

private:
	PlaylistDAO playlist_dao;
	JWTUtil jwt_util;

	// 从请求中提取用户ID
	bool extractUserIdFromToken(const HttpRequest& req, int& user_id);

	// 验证用户是否有权限操作此歌单
	bool verifyPlaylistOwnership(int playlist_id, int user_id);

	// 从URL中提取ID
	int extractIdFromPath(const std::string& path, const std::string& pattern);
};