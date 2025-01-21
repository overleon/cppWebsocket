#pragma once
#include <cstdint>
#include <algorithm>
#include "overHere/error/propagation.hpp"

namespace overHere::net::ws
{
    using Error_ptr = error::Error_ptr;
    enum error_e : uint32_t
    {
        ERR_ID,
        ERR_WS_CONNECTING,
        ERR_WS_CLOSING,
        ERR_WS_SENDING_MSG,
        ERR_WS_MODULE_NOT_INITIALIZED,
        ERR_WS_NOT_STOPPED,
        ERR_WS_STD_CONTAINER,
        ERR_WS_CONNECTION_NO_FOUND,
        ERR_WS_STREAMER_CONNECTING,
        ERR_WS_STREAMER_NOT_CONNECTED,
        ERR_WS_STREAMER_TOPICS_REJECTED,
        ERR_WS_STREAMER_CLOSING,
        ERR_WS_STREAMER_USED,
        ERR_WS_STREAMER_MAX_SUBSCRIPTIONS,
        ERR_WS_STREAMER_IN_FAILED_STATE,
        ERR_NUM,
    };

    inline constexpr const char *err_msg[error_e::ERR_NUM] = {
        /*ERR_ID*/ "exch-net",
        /*ERR_WS_CONNECTING*/ "Error connecting",
        /*ERR_WS_CLOSING*/ "Error closing connection",
        /*ERR_WS_SENDING_MSG*/ "Error sending msg",
        /*ERR_WS_MODULE_NOT_INITIALIZED*/ "Module has not been initialized",
        /*ERR_WS_NOT_STOPPED*/ "The connections have not been stopped",
        /*ERR_WS_STD_CONTAINER*/ "Error Emplacing new item to map",
        /*ERR_WS_CONNECTION_NO_FOUND*/ "The connection was not found",
        /*ERR_WS_STREAMER_CONNECTING*/ "The streamer was connecting",
        /*ERR_WS_STREAMER_NOT_CONNECTED*/ "The streamer not connected",
        /*ERR_WS_STREAMER_TOPICS_REJECTED*/ "Some topics rejected to subscribe",
        /*ERR_WS_STREAMER_CLOSING*/ "The streamer is being closed",
        /*ERR_WS_STREAMER_USED*/ "The streamer was already used, check status.",
        /*ERR_WS_STREAMER_MAX_SUBSCRIPTIONS*/ "Max number of subscription achieved",
        /*ERR_WS_STREAMER_IN_FAILED_STATE*/ "Connection in falied state",
    };
}