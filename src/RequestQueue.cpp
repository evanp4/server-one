#include "RequestQueue.h"

void RequestQueue::push(std::shared_ptr<tcp::socket> socket) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(socket);
    condition_.notify_one();
}

std::shared_ptr<tcp::socket> RequestQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return !queue_.empty(); });
    auto socket = queue_.front();
    queue_.pop();
    return socket;
}
