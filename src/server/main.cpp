#include "Server.h"
#include "../database/DBManager.h"
#include "../utils/JsonUtil.h"
#include "../utils/Config.h"

#include <csignal>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

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
	server.addRouter(http::verb::get, "/", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(), "Welcome to HW Music Player!");
	});

	server.addRouter(http::verb::post, "/", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(), "Welcome to HW Music Player!");
	});

	// 处理器

	// 用户路由
	// 用户注册 	POST /users/register
	// 用户登录 	POT /users/login
	// 用户信息 	GET /users/:id
	// 用户信息更新  PUT /users/:id
	// 用户密码更新  PUT /users/:id/password

	// 歌单路由
	// 歌单列表 		GET /playlists
	// 歌单所有歌曲 	 GET /playlists/:id/songs
	// 歌单添加歌曲 	 POST /playlists/:id/songs
	// 歌单删除歌曲 	 DELETE /playlists/:id/songs/:song_id
	// 歌单创建 		POST /playlists
	// 歌单删除 		DELETE /playlists/:id

	// 播放历史路由
	// 播放历史所有歌曲 	GET /history
	// 播放历史添加歌曲 	POST /history
	// 播放历史删除歌曲 	DELETE /history/:id
	// 播放历史清空 		DELETE /history

	// 音乐推荐
}