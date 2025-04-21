#pragma once
#include "Router.h"

#include <memory>

#include <boost/beast/core/flat_buffer.hpp>

/**
 * @brief HttpSession 单个连接会话管理
 */
class Session : public std::enable_shared_from_this<Session> {
public:
	Session(tcp::socket socket, Router& router);

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