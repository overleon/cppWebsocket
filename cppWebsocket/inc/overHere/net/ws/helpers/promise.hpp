#pragma once

#include <chrono>
#include <future>
#include <array>
#include <string_view>

namespace overHere::net::ws
{
    struct future_t;

    struct promise_t
    {
        public:    
            enum error_e
            {
                OK,
                ERROR_SUBSCRIBING,
                ERROR_TIMEOUT,
                ERROR_PROMISE,
                NUM,
            };
            
        private:
            std::chrono::steady_clock::time_point _timeout;
            std::promise<error_e> _promise;
            static std::array<std::string_view, error_e::NUM> _err_msg;

        public:
            promise_t(future_t &);
            error_e Set(error_e);
            static std::string_view Err_msg(error_e  err);
    };

    struct future_t
    {
    private:
        friend struct promise_t;
        std::future<promise_t::error_e> _future;
        std::chrono::steady_clock::time_point _timeout;

    public:
        promise_t::error_e Get();
    };
}