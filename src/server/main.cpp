#include "Server.h"
#include "../database/DBManager.h"
#include "../utils/JsonUtil.h"
#include "../utils/Config.h"
#include "../handlers/UserHandler.h"

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <csignal>
#include <string>

Server* g_server = nullptr;

void setupRoutes(Server& server);

void signalHandler(int signo) {
	std::cout << "Received signal " << signo << ", stopping server..." << "\n";
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
	// 处理器、中间件
	auto user_handler = std::make_shared<UserHandler>(Config::getInstance()->getJWTConfig().secret);

	server.addRouter(http::verb::get, "/", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(
			request.version(), json{{"message", "Welcome to HarmonyOS Music Player!"}}.dump());
	});

	/*********************************** 用户路由 ****************************************/
	// 1.用户注册			POST /users/register
	server.addRouter(http::verb::post, "/users/register",
					 [user_handler](const HttpRequest& request) {
						 return user_handler->handleRegister(request);
					 });

	// 2.用户获取验证码		POST /users/verify_code
	server.addRouter(http::verb::post, "/users/verify_code",
					 [user_handler](const HttpRequest& request) {
						 return user_handler->handleGetVerifyCode(request);
					 });

	// 3.用户登录 			POST /users/login
	server.addRouter(http::verb::post, "/users/login", [user_handler](const HttpRequest& request) {
		return user_handler->handleLogin(request);
	});

	// 3.获取用户信息 		GET /users/info
	server.addRouter(http::verb::get, "/users/info", [user_handler](const HttpRequest& request) {
		return user_handler->handleGetProfile(request);
	});

	// 4.修改用户头像 		POST /users/avatar
	server.addRouter(http::verb::post, "/users/avatar", [user_handler](const HttpRequest& request) {
		return user_handler->handleUpdateAvatar(request);
	});

	// 5.获取用户头像 		GET /users/avatar
	server.addRouter(http::verb::get, "/users/avatar", [user_handler](const HttpRequest& request) {
		return user_handler->handleGetAvatar(request);
	});

	// 6.修改密码 			POST /users/password
	server.addRouter(http::verb::post, "/users/password",
					 [user_handler](const HttpRequest& request) {
						 return user_handler->handleChangePassword(request);
					 });
	// 7.绑定第三方平台账号 	POST /users/bind
	server.addRouter(http::verb::post, "/users/bind", [user_handler](const HttpRequest& request) {
		return user_handler->handleBindPlatform(request);
	});

	/*********************************** 歌曲路由 ****************************************/
	// 1.歌单列表 			POST /playlists
	server.addRouter(http::verb::post, "/playlists", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"Playlists retrieved successfully"}}.dump());
	});
	// 2.歌单所有歌曲 	 	POST /playlists/songs
	server.addRouter(http::verb::post, "/playlists/songs", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"All songs in playlist retrieved"}}.dump());
	});
	// 3.歌单添加歌曲 	 	POST /playlists/add
	server.addRouter(http::verb::post, "/playlists/add", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"msg", "Song added to playlist"}}.dump());
	});
	// 4.歌单删除歌曲 	 	POST /playlists/erase
	server.addRouter(http::verb::post, "/playlists/erase", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"msg", "Song removed from playlist"}}.dump());
	});
	// 5.歌单创建 			POST /playlists/create
	server.addRouter(http::verb::post, "/playlists/create", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"Playlist created successfully"}}.dump());
	});
	// 6.歌单删除 			POST /playlists/delete
	server.addRouter(http::verb::post, "/playlists/delete", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(
			request.version(), json{{"msg", "Playlist deleted successfully"}}.dump());
	});

	/*********************************** 播放历史路由 ****************************************/
	// 1.播放历史所有歌曲	  	POST	/history
	server.addRouter(http::verb::post, "/history", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"All songs in history retrieved"}}.dump());
	});
	// 2.播放历史添加歌曲 	POST /history/add
	server.addRouter(http::verb::post, "/history/add", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"Song added to history"}}.dump());
	});
	// 3.播放历史删除歌曲 	POST /history/erase
	server.addRouter(http::verb::post, "/history/erase", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"msg", "Song removed from history"}}.dump());
	});

	// 4.播放历史清空 		POST /history/clear
	server.addRouter(http::verb::post, "/history/clear", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(), json{{"History cleared"}}.dump());
	});

	// 5.用户最常听的歌曲 GET /history/like
	server.addRouter(http::verb::post, "/history/like", [](const HttpRequest& request) {
		return JsonUtil::buildSuccessResponse(request.version(),
											  json{{"Most played songs retrieved"}}.dump());
	});
}