#include <iostream>
#include <thread>
#include <csignal>
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
using namespace std;

namespace fs = boost::filesystem;

boost::asio::io_context io_context;
unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        io_context.stop();
    }
}

void handle_connection(shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto buffer = make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(*socket, *buffer, "\r\n",
        [socket, buffer](const boost::system::error_code& error, size_t bytes_transferred) {
            if (!error) {
                istream is(buffer.get());
                string line;
                getline(is, line);
                istringstream request_line(line);
                string method;
                string uri;
                string version;
                request_line >> method >> uri >> version;
                if (uri == "/") {
                    uri = "/index.html";
                }

                string file_path = "www" + uri;
                if (fs::exists(file_path)) {
                    ifstream file(file_path);
                    stringstream response;
                    response << "HTTP/1.1 200 OK\r\n";
                    response << "Content-Type: text/html\r\n";
                    response << "Content-Length: " << fs::file_size(file_path) << "\r\n";
                    response << "\r\n";
                    response << file.rdbuf();
                    file.close();
                    boost::asio::async_write(*socket, boost::asio::buffer(response.str()),
                        [socket](const boost::system::error_code& error, size_t bytes_transferred) {
                            socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                        });
                } else {
                    string response = "HTTP/1.1 404 Not Found\r\n\r\n";
                    boost::asio::async_write(*socket, boost::asio::buffer(response),
                        [socket](const boost::system::error_code& error, size_t bytes_transferred) {
                            socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                        });
                }
            }
        });
}

void accept_connections(boost::asio::ip::tcp::acceptor& acceptor) {
    auto socket = make_shared<boost::asio::ip::tcp::socket>(io_context);
    acceptor.async_accept(*socket, [&acceptor, socket](const boost::system::error_code& error) {
        if (!error) {
            handle_connection(socket);
        }
        accept_connections(acceptor);
    });
}

int main() {
    try {
        signal(SIGINT, signal_handler);

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 12345);
        acceptor_ptr = make_unique<boost::asio::ip::tcp::acceptor>(io_context, endpoint);

        accept_connections(*acceptor_ptr);

        thread io_thread([]() {
            io_context.run();
        });

        io_thread.join();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}