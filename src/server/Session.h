#pragma once
#include "Router.h"

#include <memory>

#include <boost/beast/core/flat_buffer.hpp>

/**
 * @brief HttpSession 单个连接会话管理
 */
class Session : public std::enable_shared_from_this<Session> {
public:
	Session(const Session&) = delete;
	Session(Session&&) = delete;
	Session& operator=(const Session&) = delete;
	Session& operator=(Session&&) = delete;

	Session(tcp::socket socket, Router& router);
	~Session();

	void run();

private:
	void doRead();

	void doWrite();

	void handleRequest();

private:
	tcp::socket m_socket;
	Router& m_router;

	beast::flat_buffer m_buffer;
	HttpRequest m_request;
	HttpResponse m_response;
};