#include "Server.h"
#include "Session.h"

#include <memory>
#include <utility>

#include <boost/asio/io_context.hpp>
#include <boost/asio/socket_base.hpp>
#include <spdlog/spdlog.h>

constexpr const char* TAG = "[Server]";

Server::Server(const std::string& ip, uint16_t port, int num_threads)
	: m_addr(ip), m_port(port), m_num_threads(num_threads), m_ioc(num_threads), m_acceptor(m_ioc) {
	m_work_guards = std::make_shared<work_guard>(m_ioc.get_executor());

	tcp::endpoint ep;
	if (m_addr == "0.0.0.0")
		ep = tcp::endpoint(tcp::v4(), m_port);
	else
		ep = tcp::endpoint(net::ip::make_address(m_addr), m_port);

	m_acceptor.open(ep.protocol());
	m_acceptor.set_option(net::socket_base::reuse_address(true));
	m_acceptor.bind(ep);

	m_acceptor.listen(net::socket_base::max_listen_connections);
}

Server::~Server() { stop(); }

void Server::run() {
	m_running = true;
	doAccept();

	m_threads.reserve(m_num_threads);
	for (int i = 0; i < m_num_threads; ++i) {
		m_threads.emplace_back([this, i]() {
			spdlog::info("{} Thread {} started", TAG, i);
			m_ioc.run();
			spdlog::info("{} Thread {} stopped", TAG, i);
		});
	}

	spdlog::info("{} Server listening on {}:{}", TAG, m_addr, m_port);

	while (m_running) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void Server::stop() {
	if (!m_running) return;

	m_acceptor.close();
	m_ioc.stop();
	m_work_guards->reset();

	for (auto& thread : m_threads) {
		if (thread.joinable()) thread.join();
	}
	m_threads.clear();
	m_running = false;
	spdlog::info("{} Server stopped", TAG);
}

void Server::doAccept() {
	if (!m_running) return;

	m_acceptor.async_accept([this](const boost::system::error_code& ec, tcp::socket socket) {
		if (ec) {
			spdlog::error("{} Accept error: {}", TAG, ec.message());
		} else {
			std::make_shared<Session>(std::move(socket), m_router)->run();
		}

		// 继续接受下一个连接
		doAccept();
	});
}
