#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

namespace net = boost::asio;
namespace http = boost::beast::http;
namespace beast = boost::beast;
using tcp = net::ip::tcp;
using json = nlohmann::json;
using work_guard = net::executor_work_guard<net::io_context::executor_type>;

using HttpRequest = http::request<http::string_body>;
using HttpResponse = http::response<http::string_body>;
