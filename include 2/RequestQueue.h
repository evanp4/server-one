#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class RequestQueue {
public:
    void push(std::shared_ptr<tcp::socket> socket);
    std::shared_ptr<tcp::socket> pop();

private:
    std::queue<std::shared_ptr<tcp::socket>> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
};
