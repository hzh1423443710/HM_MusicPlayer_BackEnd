#include "Server.h"
#include "../database/DBManager.h"
#include "../utils/JsonUtil.h"
#include "../utils/Config.h"
#include "../handlers/UserHandler.h"

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <csignal>
#include <regex>
#include <string>
#include <vector>

Server* g_server = nullptr;

void setupRoutes(Server& server);

void signalHandler(int signal) {
	if (g_server != nullptr) {
		g_server->stop();
	}
}

int main() {
	signal(SIGINT, signalHandler);

	try {
		Config* config = Config::getInstance();
		if (!config->loadFromFile("config.json")) {
			spdlog::error("Failed to load config file");
			return 1;
		}

		spdlog::set_level(spdlog::level::from_str(config->getLogConfig().level));
		DBManager::init(config->getDatabaseConfig().host, config->getDatabaseConfig().port,
						config->getDatabaseConfig().user, config->getDatabaseConfig().password,
						config->getDatabaseConfig().dbname,
						config->getDatabaseConfig().connection_pool_size);

		Server server(config->getServerConfig().host, config->getServerConfig().port,
					  config->getServerConfig().threads);
		g_server = &server;
		setupRoutes(server);
		server.run();

	} catch (const std::exception& e) {
		spdlog::error("Exception: {}", e.what());
		return 1;
	}
	return 0;
}

void setupRoutes(Server& server) {
	// 处理器
	auto user_handler = std::make_shared<UserHandler>(Config::getInstance()->getJWTConfig().secret);

	server.addRouter(http::verb::get, std::regex("/?"), [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(
			request.version(), json{{"message", "Welcome to HarmonyOS Music Player!"}}.dump());
	});

	/**
	 * @brief 用户路由
	 */
	// 用户注册 	POST /users/register
	server.addRouter(http::verb::post, std::regex("/users/register\\s*"),
					 [user_handler](const HttpRequest& request) {
						 return user_handler->handleRegister(request);
					 });
	// 用户登录 	POST /users/login
	server.addRouter(
		http::verb::post, std::regex("/users/login\\s*"),
		[user_handler](const HttpRequest& request) { return user_handler->handleLogin(request); });
	// 用户信息 	GET /users/{id}
	server.addRouter(http::verb::get, std::regex("/users/[0-9]+\\s*"),
					 [](const HttpRequest& request) {
						 auto segs = JsonUtil::getPathParams(request.target());

						 json j{{"id", segs[1]}, {"msg", "OK"}};

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });
	// 用户信息更新  PUT /users/{id}
	server.addRouter(http::verb::put, std::regex("/users/[0-9]+\\s*"),
					 [](const HttpRequest& request) {
						 auto segs = JsonUtil::getPathParams(request.target());

						 json j{{"id", segs[1]}, {"msg", "User info updated successfully"}};

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });
	// 修改密码 PUT /users/{id}/password
	server.addRouter(
		http::verb::put, std::regex("/users/[0-9]+/password\\s*"), [](const HttpRequest& request) {
			return JsonUtil::buildSuccessResponse(
				request.version(), json{{"message", "Password updated successfully"}}.dump());
		});

	/**
	 * @brief 歌单路由
	 */
	// 歌单列表 		GET /playlists
	server.addRouter(http::verb::get, std::regex("/playlists\\s*"), [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"Playlists retrieved successfully"}}.dump());
	});
	// 歌单所有歌曲 	 GET /playlists/{id}/songs
	server.addRouter(http::verb::get, std::regex("/playlists/[0-9]+/songs\\s*"),
					 [](const HttpRequest& request) {
						 auto segs = JsonUtil::getPathParams(request.target());
						 json j{{"id", segs[1]}, {"msg", "All songs in playlist retrieved"}};

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });
	// 歌单添加歌曲 	 POST /playlists/{id}/songs
	server.addRouter(http::verb::post, std::regex("/playlists/[0-9]+/songs\\s*"),
					 [](const HttpRequest& request) {
						 auto segs = JsonUtil::getPathParams(request.target());
						 json j{{"id", segs[1]}, {"msg", "Song added to playlist"}};

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });
	// 歌单删除歌曲 	 DELETE /playlists/{id}/songs/{song_id}
	server.addRouter(http::verb::delete_, std::regex("/playlists/[0-9]+/songs/[0-9]+\\s*"),
					 [](const HttpRequest& request) {
						 auto segs = JsonUtil::getPathParams(request.target());
						 json j{{"playlist_id", segs[1]},
								{"song_id", segs[3]},
								{"msg", "Song removed from playlist"}};

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });
	// 歌单创建 		POST /playlists
	server.addRouter(http::verb::post, std::regex("/playlists\\s*"),
					 [](const HttpRequest& request) {
						 return JsonUtil::buildSuccessResponse(
							 request.version(), json{{"Playlist created successfully"}}.dump());
					 });
	// 歌单删除 		DELETE /playlists/{id}
	server.addRouter(http::verb::delete_, std::regex("/playlists/[0-9]+\\s*"),
					 [](const HttpRequest& request) {
						 auto segs = JsonUtil::getPathParams(request.target());
						 json j{{"id", segs[1]}, {"msg", "Playlist deleted successfully"}};

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });

	/**
	 * @brief 播放历史路由
	 */
	// 播放历史所有歌曲 	GET /history
	server.addRouter(http::verb::get, std::regex("/history\\s*"), [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"All songs in history retrieved"}}.dump());
	});
	// 播放历史添加歌曲 	POST /history
	server.addRouter(http::verb::post, std::regex("/history\\s*"), [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"Song added to history"}}.dump());
	});
	// 播放历史删除歌曲 	DELETE /history/{id}
	server.addRouter(http::verb::delete_, std::regex("/history/[0-9]+\\s*"),
					 [](const HttpRequest& request) {
						 auto segs = JsonUtil::getPathParams(request.target());
						 json j{{"id", segs[1]}, {"msg", "Song removed from history"}};

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });
	// 播放历史清空 		DELETE /history
	server.addRouter(http::verb::delete_, std::regex("/history\\s*"),
					 [](const HttpRequest& request) {
						 return JsonUtil::buildSuccessResponse(request.version(),
															   json{{"History cleared"}}.dump());
					 });

	// 用户最常听的歌曲
	server.addRouter(http::verb::get, std::regex("/history/most_played(\\?.*)?\\s*"),
					 [](const HttpRequest& request) {
						 // Get query parameters
						 auto queryParams = JsonUtil::getQueryParams(request.target());
						 // Example: access limit parameter - /history/most_played?limit=10
						 int limit = 10; // default value
						 if (queryParams.find("limit") != queryParams.end()) {
							 limit = std::stoi(queryParams["limit"]);
						 }

						 json j;
						 for (auto& kv : queryParams) {
							 j[kv.first] = kv.second;
						 }
						 j["msg"] = "Most played songs retrieved successfully";

						 return JsonUtil::buildSuccessResponse(request.version(), j.dump());
					 });
}