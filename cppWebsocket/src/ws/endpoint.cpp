#include "overHere/net/ws/ws/endpoint.hpp"

namespace overHere::net::ws
{
    endpoint_c::endpoint_c()
        : m_next_id(0),
          _all_closed_status(false)
    {
        // Disable all logging
        _endpoint.clear_access_channels(websocketpp::log::alevel::all);
        
        // Optionally, enable only error logging
        // _endpoint.set_access_channels(websocketpp::log::alevel::fail);

        _endpoint.init_asio();
        _endpoint.start_perpetual();

        m_thread.reset(new websocketpp::lib::thread(&client::run, &_endpoint));
    }

    endpoint_c::~endpoint_c()
    {
        auto err = Close_all_connections();
        if (err)
            std::cout << err->Error_msg() << std::endl;

        if (m_thread->joinable())
            m_thread->join();
    }

    Error_ptr endpoint_c::Wait_for_closing_all(void)
    {
        std::lock_guard guard(_mtx);
        if (_all_closed_status == false)
            return OVERH_SP_ERR(ERR_WS_NOT_STOPPED, err_msg);
        if (_connection_map.empty())
            return nullptr;
        if (m_thread->joinable())
            m_thread->join();
        return nullptr;
    }

    Error_ptr endpoint_c::Close_all_connections(void)
    {
        std::unique_lock guard(_mtx);
        if (_all_closed_status)
            return nullptr;
        Error_ptr err{nullptr};
        _endpoint.stop_perpetual();
        for (auto it = _connection_map.begin(); it != _connection_map.end(); ++it)
        {
            if (it->second->Status() != metadata_c::status_e::OPENED)
                continue;

            websocketpp::lib::error_code ec;
            _endpoint.close(it->second->_hdl, websocketpp::close::status::normal, "", ec);
            if (ec && !err)
                err = OVERH_SP_ERR_DT(ERR_WS_CLOSING, err_msg, ec.message());
        }
        _all_closed_status = true;
        return err;
    }

    endpoint_c::context_ptr_a endpoint_c::_On_tls_init()
    {
        context_ptr_a ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
        try
        {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3 |
                             boost::asio::ssl::context::single_dh_use);
        }
        catch (std::exception &e)
        {
            std::cout << "Error in context pointer: " << e.what() << std::endl;
        }
        return ctx;
    }

    Error_ptr endpoint_c::Create_new_connection(const std::string &uri,
                                                std::future<metadata_c::status_e> &future,
                                                int &id,
                                                metadata_c::on_callbacks_t &on_callback)
    {
        std::lock_guard guard(_mtx);

        websocketpp::lib::error_code ec;

        _endpoint.set_tls_init_handler(websocketpp::lib::bind(&endpoint_c::_On_tls_init));
        client::connection_ptr con = _endpoint.get_connection(uri, ec);

        if (ec)
            return OVERH_SP_ERR_DT(ERR_WS_CONNECTING, err_msg, ec.message());

        id = m_next_id++;
        auto metadata = new metadata_c(uri, future, id, on_callback, con->get_handle());
        metadata_c::ptr_a metadata_ptr(metadata);
        _connection_map[id] = metadata_ptr;

        if (metadata->_on_callbacks.on_msg)
        {
            con->set_message_handler([this, metadata](websocketpp::connection_hdl hdl, client::message_ptr msg)
                                     { metadata->_on_callbacks.on_msg(this->_endpoint, hdl, msg, _json_parser); });
        }

        con->set_open_handler([this, metadata](websocketpp::connection_hdl hdl)
                              { metadata->On_open(this->_endpoint, hdl); });

        con->set_fail_handler([this, metadata](websocketpp::connection_hdl hdl)
                              { metadata->On_fail(this->_endpoint, hdl); });

        con->set_close_handler([this, metadata](websocketpp::connection_hdl hdl)
                               { metadata->On_close(this->_endpoint, hdl); });

        _endpoint.connect(con);

        return nullptr;
    }

    Error_ptr endpoint_c::Close(int id,
                                websocketpp::close::status::value code)
    {
        std::lock_guard guard(_mtx);
        auto metadata_it = _connection_map.find(id);
        if (metadata_it == _connection_map.end())
            return OVERH_SP_ERR(ERR_WS_CONNECTION_NO_FOUND, err_msg);

        websocketpp::lib::error_code ec;
        _endpoint.close(metadata_it->second->_hdl, code, "", ec);
        if (ec)
            return OVERH_SP_ERR_DT(ERR_WS_CLOSING, err_msg, ec.message());
        metadata_it->second->_status = metadata_c::status_e::CLOSING;
        return nullptr;
    }

    Error_ptr endpoint_c::Send_msg(int id,
                                   const std::string &msg)
    {
        std::lock_guard guard(_mtx);
        auto metadata_it = _connection_map.find(id);
        if (metadata_it == _connection_map.end())
            return OVERH_SP_ERR(ERR_WS_CONNECTION_NO_FOUND, err_msg);
        websocketpp::lib::error_code ec;
        _endpoint.send(metadata_it->second->_hdl, msg,
                       websocketpp::frame::opcode::TEXT, ec);
        if (ec)
            return OVERH_SP_ERR_DT(ERR_WS_SENDING_MSG, err_msg, ec.message());
        return nullptr;
    }

    Error_ptr endpoint_c::Disable_callbacks(int id)
    {
        std::lock_guard guard(_mtx);
        auto metadata_it = _connection_map.find(id);
        if (metadata_it == _connection_map.end())
            return OVERH_SP_ERR(ERR_WS_CONNECTION_NO_FOUND, err_msg);
        metadata_it->second->Disable_all_callbacks();
        return nullptr;
    }
}