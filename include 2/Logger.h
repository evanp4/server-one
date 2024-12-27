#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        logFile_ << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << " - " << message << std::endl;
    }

private:
    Logger() : logFile_("server.log", std::ios_base::app) {}
    ~Logger() { logFile_.close(); }

    std::ofstream logFile_;
    std::mutex mutex_;
};
