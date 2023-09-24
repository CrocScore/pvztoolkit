#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <string>
#include <thread> // For multi-threading
#include "HttpServer.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

HttpServer::HttpServer(const std::string& address, unsigned short port)
    : acceptor_(ioc_, {net::ip::make_address(address), port}) {}

void HttpServer::run() {
    // Registering signal handlers for graceful shutdown
    boost::asio::signal_set signals(ioc_, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code&, int) {
        ioc_.stop();
    });

    do_accept();

    // Creating a thread pool for handling multiple requests concurrently
    const std::size_t num_threads = std::max<int>(1, std::thread::hardware_concurrency());
    threads_.reserve(num_threads - 1);

    for (std::size_t i = 0; i < num_threads - 1; ++i) {
        threads_.emplace_back([this] { ioc_.run(); });
    }

    ioc_.run();

}

void HttpServer::stop() {
    // Joining the threads after io_context stops
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void HttpServer::do_accept() {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) handle_session(std::move(socket));
        if (acceptor_.is_open()) do_accept(); // Guard against attempts to accept on a closed acceptor
    });
}

void HttpServer::handle_session(tcp::socket socket) {
    auto p_socket = std::make_shared<tcp::socket>(std::move(socket));
    http::async_read(*p_socket, buffer_, req_, 
                     [this, p_socket] 
                     (boost::system::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            std::cerr << "Read Error: " << ec.message() << "\n";
            return;
        }

        auto path = std::string(req_.target());
        auto response = handle_request(path, req_.body(), req_.version());
        http::async_write(*p_socket, response, 
                          [p_socket, this] 
                          (boost::system::error_code ec, std::size_t) {
                              if (ec) std::cerr << "Write Error: " << ec.message() << "\n";
                              p_socket->close();
                              buffer_.consume(buffer_.size());
                              req_.body().clear();
                          });
    });
}

http::response<http::string_body> HttpServer::handle_request(const std::string& path, const std::string& body, unsigned version) {
    // Logic remains mostly unchanged; pass the version to response creation
    auto webhookId = path.substr(path.rfind('/') + 1);

    std::cout << "Received Webhook " << webhookId << " Payload: " << body << "\n";

    if (webhookId == "follow")
        std::cout << "User has followed the livestream\n";
    else if (webhookId == "like")
        std::cout << "User has liked the livestream\n";
    else if (webhookId == "share")
        std::cout << "User has shared the livestream\n";
    else {
        std::cout << "Unknown webhook ID: " << webhookId << "\n";
        return http::response<http::string_body>{http::status::bad_request, 11, "Bad Request"};

    }

     return http::response<http::string_body>{http::status::ok, version, ""};
}

int main2() {
    try {
        HttpServer server{"0.0.0.0", 3000}; // Listen on all available network interfaces.
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}