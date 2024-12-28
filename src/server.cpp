#include <iostream>
#include <thread>
#include <csignal>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
using namespace std;
namespace bs = boost::asio;
namespace fs = boost::filesystem;

bs::io_context io_context;
unique_ptr<bs::ip::tcp::acceptor> acceptor_ptr;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        io_context.stop();
    }
}

string get_random_words() {
    ifstream file("words.txt");
    vector<string> words;
    string word;
    while (file >> word) {
        words.push_back(word);
    }
    file.close();

    random_device rd;
    mt19937 gen(rd());
    shuffle(words.begin(), words.end(), gen);

    stringstream ss;
    for (int i = 0; i < 25 && i < words.size(); ++i) {
        ss << words[i] << " ";
    }
    return ss.str();
}

void handle_connection(shared_ptr<bs::ip::tcp::socket> socket) {
    auto buffer = make_shared<bs::streambuf>();
    bs::async_read_until(*socket, *buffer, "\r\n",
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
                    bs::async_write(*socket, bs::buffer(response.str()),
                        [socket](const boost::system::error_code& error, size_t bytes_transferred) {
                            socket->shutdown(bs::ip::tcp::socket::shutdown_both);
                        });
                } else if (uri == "/random-words") {
                    string random_words = get_random_words();
                    stringstream response;
                    response << "HTTP/1.1 200 OK\r\n";
                    response << "Content-Type: text/plain\r\n";
                    response << "Content-Length: " << random_words.size() << "\r\n";
                    response << "\r\n";
                    response << random_words;
                    bs::async_write(*socket, bs::buffer(response.str()),
                        [socket](const boost::system::error_code& error, size_t bytes_transferred) {
                            socket->shutdown(bs::ip::tcp::socket::shutdown_both);
                        });
                } else {
                    string response = "HTTP/1.1 404 Not Found\r\n\r\n";
                    bs::async_write(*socket, bs::buffer(response),
                        [socket](const boost::system::error_code& error, size_t bytes_transferred) {
                            socket->shutdown(bs::ip::tcp::socket::shutdown_both);
                        });
                }
            }
        });
}

void accept_connections(bs::ip::tcp::acceptor& acceptor) {
    auto socket = make_shared<bs::ip::tcp::socket>(io_context);
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

        bs::ip::tcp::endpoint endpoint(bs::ip::tcp::v4(), 12345);
        acceptor_ptr = make_unique<bs::ip::tcp::acceptor>(io_context, endpoint);

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