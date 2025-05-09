#pragma once
#include "../common/net.h"

#include <boost/beast/http/status.hpp>

#include <string>

class JsonUtil {
public:
	static HttpResponse buildErrorResponse(const http::status& status, unsigned int version,
										   const std::string& message = "Internal Server Error");

	static HttpResponse buildSuccessResponse(unsigned int version, const std::string& json_msg);
};

// void connection::file_reply(core::string_view path) {
// 	http::file_body::value_type body;
// 	std::string jpath = path_cat(doc_root, path);
// 	body.open(jpath.c_str(), beast::file_mode::scan, ec);
// 	if (ec == beast::errc::no_such_file_or_directory) {
// 		error_reply(http::status::not_found,
// 					"The resource '" + std::string(path) + "' was not found in " + jpath);
// 		return;
// 	}
// 	auto const size = body.size();
// 	http::response<http::file_body> res{std::piecewise_construct,
// std::make_tuple(std::move(body)),
// std::make_tuple(http::status::ok, req.version())}; 	res.set(http::field::server,
// BOOST_BEAST_VERSION_STRING); 	res.set(http::field::content_type, mime_type(path));
// 	res.content_length(size);
// 	res.keep_alive(req.keep_alive());
// 	http::write(socket, res, ec);
// }