#pragma once

#include <vector>
#include <string>
#include <future>
#include <atomic>
#include "simdjson.h"

#include "websocketpp/config/asio_client.hpp"
#include "websocketpp/client.hpp"
#include "websocketpp/common/thread.hpp"
#include "websocketpp/common/memory.hpp"

namespace overHere::net::ws
{
    class metadata_c
    {
    protected:
    private:
    public:
        /*PUBLIC ALIAS*/
        using client = websocketpp::client<websocketpp::config::asio_tls_client>;
        using context_ptr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;
        using ptr_a = websocketpp::lib::shared_ptr<metadata_c>;
        using on_open_a = std::function<void(client &, websocketpp::connection_hdl)>;
        using on_close_a = std::function<void(uint16_t, std::string_view)>;
        using on_fail_a = std::function<void(uint16_t, std::string_view)>;

        using on_msg_a = std::function<void(client &, websocketpp::connection_hdl,
                                            client::message_ptr, simdjson::ondemand::parser &)>;

    protected:
    private:
    public:
        /*PUBLIC STRUCTS*/
        struct on_callbacks_t
        {
            on_open_a on_open{nullptr};
            on_close_a on_close{nullptr};
            on_fail_a on_fail{nullptr};
            on_msg_a on_msg{nullptr};
        };

        enum status_e
        {
            CONNECTING,
            OPENED,
            FAILED,
            CLOSED,
            CLOSING,
        };

    protected:
    private:
        friend class endpoint_c;

        std::vector<std::string> _history_msg;
        std::string _uri;
        std::atomic<uint8_t> _status;
        std::string _server;
        std::string _error_reason;
        websocketpp::connection_hdl _hdl;
        std::promise<status_e> _promise;
        int _id;

    public:
        metadata_c(const std::string &,
                   std::future<status_e> &,
                   int,
                   const on_callbacks_t &,
                   websocketpp::connection_hdl);

        on_callbacks_t _on_callbacks;
        void On_open(client &, websocketpp::connection_hdl);
        void On_close(client &, websocketpp::connection_hdl);
        void On_fail(client &, websocketpp::connection_hdl);
        void Disable_all_callbacks(void);
        inline status_e Status() const
        {
            return static_cast<status_e>(_status.load());
        }

        inline int64_t Id() const { return _id; }
    };
}