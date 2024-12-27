#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <boost/asio.hpp>
using namespace std;

using boost::asio::ip::tcp;

class RequestQueue {
public:
    void push(shared_ptr<tcp::socket> socket);
    shared_ptr<tcp::socket> pop();

private:
    queue<shared_ptr<tcp::socket>> queue_;
    mutex mutex_;
    condition_variable condition_;
};
