#include <iostream>
#include <thread>
#include <csignal>
#include <boost/asio.hpp>

boost::asio::io_context io_context;
std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "SIGINT received, stopping server..." << std::endl;
        io_context.stop();
    }
}

void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(*socket, *buffer, "\r\n",
        [socket, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (!error) {
                std::istream is(buffer.get());
                std::string line;
                std::getline(is, line);
                std::cout << "Received: " << line << std::endl;

                // Send a response
                std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
                boost::asio::async_write(*socket, boost::asio::buffer(response),
                    [socket](const boost::system::error_code& error, std::size_t) {
                        if (!error) {
                            socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                            socket->close();
                        }
                    });
            }
        });
}

void accept_connections(boost::asio::ip::tcp::acceptor& acceptor) {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    acceptor.async_accept(*socket, [&acceptor, socket](const boost::system::error_code& error) {
        if (!error) {
            std::cout << "Accepted connection" << std::endl;
            handle_connection(socket);
        }
        accept_connections(acceptor); // Accept the next connection
    });
}

int main() {
    try {
        // Register signal handler
        std::signal(SIGINT, signal_handler);

        // Server setup code
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 12345);
        acceptor_ptr = std::make_unique<boost::asio::ip::tcp::acceptor>(io_context, endpoint);

        accept_connections(*acceptor_ptr);

        std::thread io_thread([]() {
            io_context.run();
        });

        io_thread.join();
        std::cout << "Server shutting down..." << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}