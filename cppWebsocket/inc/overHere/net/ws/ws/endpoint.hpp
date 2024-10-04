#pragma once
#include <map>
#include "simdjson.h"

#include "overHere/net/ws/ws/metadata.hpp"
#include "overHere/net/ws/error.hpp"

namespace overHere::net::ws
{
    class endpoint_c
    {
    private:
        /*PRIVATE ALIAS*/
        using client = websocketpp::client<websocketpp::config::asio_tls_client>;

    public:
        /*PUBLIC ALIAS*/
        using context_ptr_a = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;
        using on_msg_a = std::function<void(client *, websocketpp::connection_hdl)>;

    protected:
    private:
        using conn_map_a = std::map<int, metadata_c::ptr_a>;

        websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

        simdjson::ondemand::parser _json_parser;
        conn_map_a _connection_map;
        int m_next_id;
        client _endpoint;
        std::mutex _mtx;
        bool _all_closed_status;

        static context_ptr_a _On_tls_init();

    public:
        endpoint_c();
        ~endpoint_c();

        Error_ptr Create_new_connection(const std::string &,
                                        std::future<metadata_c::status_e> &,
                                        int &,
                                        metadata_c::on_callbacks_t &);
        Error_ptr Close(int,
                        websocketpp::close::status::value = websocketpp::close::status::normal);

        Error_ptr Send_msg(int,
                           const std::string &);

        Error_ptr Close_all_connections(void);

        Error_ptr Wait_for_closing_all(void);

        Error_ptr Disable_callbacks(int id);
    };

}