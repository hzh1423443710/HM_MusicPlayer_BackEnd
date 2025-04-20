// HistoryHandler.h
#pragma once
// #include "../database/PlayHistoryDAO.h"
#include "../utils/JWTUtil.h"
#include "../common/net.h"

// class HistoryHandler {
// public:
// 	HistoryHandler(const std::string& jwt_secret);

// 	// 记录播放历史
// 	HttpResponse handleAddHistory(const HttpRequest& req);

// 	// 获取播放历史
// 	HttpResponse handleGetHistory(const HttpRequest& req);

// 	// 清空播放历史
// 	HttpResponse handleClearHistory(const HttpRequest& req);

// 	// 获取播放统计
// 	HttpResponse handleGetStats(const HttpRequest& req);

// private:
// 	PlayHistoryDAO history_dao;
// 	JWTUtil jwt_util;

// 	// 从请求中提取用户ID
// 	bool extractUserIdFromToken(const HttpRequest& req, int& user_id);
// };