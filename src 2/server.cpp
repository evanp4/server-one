#include <iostream>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <csignal>
#include "ThreadPool.h"
#include "RequestQueue.h"
#include "Logger.h"
#include "TypingTest.h"

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
        TypingTest typingTest("words.txt");
        std::string sentence = typingTest.getRandomSentence(5); // Generate a sentence with 5 random words
        Logger::getInstance().log("Generated sentence: " + sentence);
        boost::asio::write(*socket, boost::asio::buffer(sentence + "\n"));

        std::array<char, 1024> buffer;
        boost::system::error_code error;

        size_t len = socket->read_some(boost::asio::buffer(buffer), error);
        if (error) {
            Logger::getInstance().log("Error reading from socket: " + error.message());
            return;
        }

        std::string user_input(buffer.data(), len);
        Logger::getInstance().log("Received input: " + user_input);

        if (user_input == sentence) {
            boost::asio::write(*socket, boost::asio::buffer("Correct! Well done.\n"));
        } else {
            boost::asio::write(*socket, boost::asio::buffer("Incorrect. Try again.\n"));
        }
    } catch (std::exception& e) {
        Logger::getInstance().log(std::string("Exception: ") + e.what());
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), PORT));

        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        Logger::getInstance().log("Server is running on port " + std::to_string(PORT));

        while (running) {
            std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);
            std::thread(handle_client, socket).detach();
        }
    } catch (std::exception& e) {
        Logger::getInstance().log(std::string("Exception: ") + e.what());
    }

    Logger::getInstance().log("Server has shut down.");
    return 0;
}