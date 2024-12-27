#include "RequestQueue.h"
using namespace std;

void RequestQueue::push(shared_ptr<tcp::socket> socket) {
    lock_guard<mutex> lock(mutex_);
    queue_.push(socket);
    condition_.notify_one();
}

shared_ptr<tcp::socket> RequestQueue::pop() {
    unique_lock<mutex> lock(mutex_);
    condition_.wait(lock, [this] { return !queue_.empty(); });
    auto socket = queue_.front();
    queue_.pop();
    return socket;
}
