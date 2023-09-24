#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#pragma once // Or use include guards
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class HttpServer {
public:
    HttpServer(const std::string& address, unsigned short port);
    void run();
    void stop();
private:
    void do_accept();
    void handle_session(tcp::socket socket);
    http::response<http::string_body> handle_request(const std::string& path, const std::string& body, unsigned version);
    
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::vector<std::thread> threads_;
};

#endif // HTTP_SERVER_H
