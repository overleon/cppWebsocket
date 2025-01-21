#include <iostream>
#include "overHere/net/ws/ws/endpoint_pool.hpp"

using namespace overHere::net::ws;

std::promise<bool> echo_got;
std::future<bool> echo_got_future = echo_got.get_future();

static void open_callback(metadata_c::client &client,
                          websocketpp::connection_hdl conn_hdl)
{
    std::cout << "Connection opened" << std::endl;
}

static void close_callback(uint16_t close_code,
                          std::string_view msg)
{
    std::cout << "Connection closed" << std::endl;
}

static void on_msg_callback(metadata_c::client &client,
                            websocketpp::connection_hdl conn_hdl,
                            metadata_c::client::message_ptr msg,
                            simdjson::ondemand::parser &parser)
{    
    if (msg->get_payload() == "Hello, world!")
    {
        std::cout << "Message received: " << msg->get_payload() << std::endl;
        
        echo_got.set_value(true);
    }
}

int main()
{
    
    auto &endpoint = endpoint_pool_c::Instance().Get_endpoint();
    
    metadata_c::on_callbacks_t callbacks = 
    {
        .on_open = open_callback,   
        .on_close = close_callback, 
        .on_msg = on_msg_callback,  
    };

    int id_connection; 
    std::future<metadata_c::status_e> future; 
    
    auto err = endpoint.Create_new_connection("wss://echo.websocket.org",
                                              future,
                                              id_connection,
                                              callbacks);
    
    if (err)
        throw std::runtime_error(std::string(err->Error_msg()));
    
    auto status = future.get();
    if (status != metadata_c::status_e::OPENED)
        throw std::runtime_error("Connection failed");

    std::cout << "Connection successful" << std::endl;
    
    err = endpoint.Send_msg(id_connection, "Hello, world!");
    if (err)
        throw std::runtime_error(std::string(err->Error_msg()));

    std::cout << "Message sent" << std::endl;
    
    echo_got_future.wait();
    
    err = endpoint.Close(id_connection);
    if (err)
        throw std::runtime_error(std::string(err->Error_msg()));

    return 0; 
}