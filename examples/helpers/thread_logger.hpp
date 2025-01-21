#pragma once
#include <mutex>
#include <iostream>

class ThreadSafeLog
{
public:
    static ThreadSafeLog &Instance()
    {
        static ThreadSafeLog instance;
        return instance;
    }

    template <typename... Args>
    void Log(Args &&...args)
    {
        std::lock_guard guard(_mutex);
        (std::cout << ... << args) << std::endl;
    }

    void LogStream(const std::string &msg)
    {
        std::lock_guard guard(_mutex);
        std::cout<<msg<<std::endl;
    }

private:
    ThreadSafeLog() = default;
    ~ThreadSafeLog() = default;

    ThreadSafeLog(const ThreadSafeLog &) = delete;
    ThreadSafeLog &operator=(const ThreadSafeLog &) = delete;

    std::mutex _mutex;
};