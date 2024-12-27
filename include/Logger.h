#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>
using namespace std;

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void log(const string& message) {
        lock_guard<mutex> lock(mutex_);
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);
        logFile_ << put_time(localtime(&in_time_t), "%Y-%m-%d %X") << " - " << message << endl;
    }

private:
    Logger() : logFile_("server.log", ios_base::app) {}
    ~Logger() { logFile_.close(); }

    ofstream logFile_;
    mutex mutex_;
};
