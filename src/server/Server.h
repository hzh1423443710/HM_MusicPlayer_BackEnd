#pragma once
#include "Router.h"

#include <memory>
#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/beast/http/verb.hpp>

/**
 * @brief HttpServer
 */
class Server {
public:
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	Server(const std::string& ip, uint16_t port, int num_threads);
	~Server();

	void run();

	void stop();

	void addRouter(const http::verb& method, const std::regex& path_regex, RouterHandler handler) {
		m_router.addRouter(method, path_regex, std::move(handler));
	}

private:
	// 异步接受连接
	void doAccept();

private:
	std::string m_addr;
	uint16_t m_port;
	int m_num_threads;

	net::io_context m_ioc;
	std::vector<std::thread> m_threads;
	std::shared_ptr<work_guard> m_work_guards;

	tcp::acceptor m_acceptor;

	Router m_router;
	std::atomic<bool> m_running{false};
};