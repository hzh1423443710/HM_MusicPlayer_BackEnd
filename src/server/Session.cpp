#include "Session.h"
#include "../utils/JsonUtil.h"

#include <boost/beast/core/error.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/write.hpp>
#include <spdlog/spdlog.h>

constexpr const char* TAG = "[Session]";

Session::Session(tcp::socket socket, Router& router)
	: m_socket(std::move(socket)), m_router{router} {}

void Session::run() { doRead(); }

void Session::doRead() {
	auto self = shared_from_this();

	// TODO: 设置超时

	http::async_read(m_socket, m_buffer, m_request,
					 [self](const beast::error_code& ec, size_t bytes_transferred) {
						 if (ec) {
							 if (ec != http::error::end_of_stream)
								 spdlog::error("{} Read error: {}", TAG, ec.message());
							 return;
						 }

						 self->handleRequest();
						 self->doWrite();
					 });
}

void Session::doWrite() {
	auto self = shared_from_this();
	// TODO: 设置超时

	http::async_write(m_socket, m_response,
					  [self](const beast::error_code& ec, size_t bytes_transferred) {
						  if (ec) {
							  spdlog::error("{} Write error: {}", TAG, ec.message());
							  return;
						  }

						  // 保持连接
						  if (self->m_response.need_eof()) {
							  beast::error_code ec;
							  self->m_socket.shutdown(tcp::socket::shutdown_send, ec);
							  return;
						  }

						  // 继续读取下一个请求
						  self->doRead();
					  });
}

void Session::handleRequest() {
	try {
		m_response = m_router.router(m_request);
		spdlog::info("{} {} {}", TAG, m_request.method_string(), m_request.target());
	} catch (const std::exception& e) {
		// 500 错误
		spdlog::error("{} Handle Request: {}", TAG, e.what());
		m_response = JsonUtil::buildErrorResponse(http::status::internal_server_error,
												  m_request.version(), e.what());
	}
}
