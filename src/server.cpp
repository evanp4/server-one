#include <iostream>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <csignal>
#include "ThreadPool.h"
#include "RequestQueue.h"
#include "Logger.h"

using boost::asio::ip::tcp;

const int PORT = 8080;
bool running = true;

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        running = false;
        Logger::getInstance().log("Received signal to terminate. Shutting down...");
    }
}

void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        std::array<char, 1024> buffer;
        boost::system::error_code error;

        while (true) {
            size_t len = socket->read_some(boost::asio::buffer(buffer), error);
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            // Create a simple HTTP response
            std::string response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "\r\n"
                                   "Hello, World!";

            boost::asio::write(*socket, boost::asio::buffer(response));

            // Log the request and response
            Logger::getInstance().log("Handled request from " + socket->remote_endpoint().address().to_string());
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
        Logger::getInstance().log("Exception in thread: " + std::string(e.what()));
    }
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), PORT));
        ThreadPool pool(4); // Create a thread pool with 4 threads
        RequestQueue requestQueue;

        std::cout << "Server is running on port " << PORT << std::endl;
        Logger::getInstance().log("Server started on port " + std::to_string(PORT));

        std::thread loadBalancer([&requestQueue, &pool]() {
            while (running) {
                auto socket = requestQueue.pop();
                pool.enqueue([socket]() {
                    handle_client(socket);
                });
            }
        });

        while (running) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);
            requestQueue.push(socket);
        }

        loadBalancer.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        Logger::getInstance().log("Exception: " + std::string(e.what()));
    }

    return 0;
}
