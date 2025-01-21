#include "overHere/net/ws/helpers/promise.hpp"

namespace overHere::net::ws
{
    using error_e = promise_t::error_e;

    std::array<std::string_view, error_e::NUM> promise_t::_err_msg =
        {
            "OK",
            "ERROR_SUBSCRIBING",
            "ERROR_TIMEOUT",
            "ERROR_PROMISE",
    };

    error_e future_t::Get(void)
    {
        if (_future.wait_until(_timeout) ==
            std::future_status::timeout) 
        {
            return error_e::ERROR_TIMEOUT;
        }
        try
        {
            return _future.get();
        }
        catch (const std::future_error &e)
        {
            return error_e::ERROR_PROMISE;
        }
        catch (...)
        {
            return error_e::ERROR_PROMISE;
        }
    }

    promise_t::promise_t(future_t &future)
    {
        auto now = std::chrono::steady_clock::now();
        _timeout = now + std::chrono::milliseconds(4910);
        future._timeout = now + std::chrono::milliseconds(5000);
        future._future = _promise.get_future();
    }

    error_e promise_t::Set(error_e value)
    {
        if (std::chrono::steady_clock::now() >= _timeout)
            return error_e::ERROR_TIMEOUT;
        try
        {
            _promise.set_value(value);
            return error_e::OK;
        }
        catch (...)
        {
            return error_e::ERROR_PROMISE;
        }
    }

    std::string_view promise_t::Err_msg(error_e err)
    {

        if (static_cast<uint8_t>(err) < error_e::NUM)
            return promise_t::_err_msg[err];
        static const std::string_view unknown = "Unknown erorr";
        return unknown;
    }
}