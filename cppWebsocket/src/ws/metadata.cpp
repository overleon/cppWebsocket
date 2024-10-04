#include "overHere/net/ws/ws/metadata.hpp"

namespace overHere::net::ws
{
    metadata_c::metadata_c(const std::string &uri,
                           std::future<status_e> &future,
                           int id,
                           const on_callbacks_t &on_callbacks,
                           websocketpp::connection_hdl hdl)
        : _on_callbacks(on_callbacks), _status(status_e::CONNECTING),
          _server("N/A"), _id(id), _uri(uri), _hdl(hdl)
    {
        future = _promise.get_future();
    }

    void metadata_c::Disable_all_callbacks()
    {
        _on_callbacks.on_close = nullptr;
        _on_callbacks.on_fail = nullptr;
        _on_callbacks.on_msg = nullptr;
        _on_callbacks.on_open = nullptr;
    }

    void metadata_c::On_open(client &c, websocketpp::connection_hdl hdl)
    {
        _status.store(status_e::OPENED);
        client::connection_ptr con = c.get_con_from_hdl(hdl);
        _server = con->get_response_header("Server");
        if (_on_callbacks.on_open)
            _on_callbacks.on_open(c, hdl);
        _promise.set_value(status_e::OPENED);
    }

    void metadata_c::On_close(client &c, websocketpp::connection_hdl hdl)
    {
        _status.store(status_e::CLOSED);
        client::connection_ptr con = c.get_con_from_hdl(hdl);
        std::stringstream s;
        s << "close code: " << con->get_remote_close_code() << " ("
          << websocketpp::close::status::get_string(con->get_remote_close_code())
          << "), close reason: " << con->get_remote_close_reason();
        _error_reason = s.str();
        if (_on_callbacks.on_close)
            _on_callbacks.on_close(con->get_remote_close_code(), _error_reason);
    }

    void metadata_c::On_fail(client &c, websocketpp::connection_hdl hdl)
    {
        _status.store(status_e::FAILED);

        client::connection_ptr con = c.get_con_from_hdl(hdl);
        _server = con->get_response_header("Server");
        _error_reason = con->get_ec().message();
        _promise.set_value(status_e::FAILED);
        if (_on_callbacks.on_fail)
            _on_callbacks.on_fail(con->get_remote_close_code(), _error_reason);
    }
}