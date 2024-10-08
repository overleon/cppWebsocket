#include <iostream>
#include "overHere/net/ws/ws/endpoint_pool.hpp"

// Use the namespace for convenience
using namespace overHere::net::ws;

// Promise and future for handling the echo response
std::promise<bool> echo_got;
std::future<bool> echo_got_future = echo_got.get_future();

// Callback function that is called when a connection is opened
static void open_callback(metadata_c::client &client,
                          websocketpp::connection_hdl conn_hdl)
{
    std::cout << "Connection opened" << std::endl;
}

// Callback function that is called when a connection is closed
static void close_callback(uint16_t close_code,
                          std::string_view msg)
{
    std::cout << "Connection closed" << std::endl;
}

// Callback function that is called when a message is received
static void on_msg_callback(metadata_c::client &client,
                            websocketpp::connection_hdl conn_hdl,
                            metadata_c::client::message_ptr msg,
                            simdjson::ondemand::parser &parser)
{
    // Check if the received message is the expected echo message
    if (msg->get_payload() == "Hello, world!")
    {
        std::cout << "Message received: " << msg->get_payload() << std::endl;
        // Set the promise value to true to indicate the message was received
        echo_got.set_value(true);
    }
}

int main()
{
    // Get the websocket endpoint instance from the endpoint pool
    auto &endpoint = endpoint_pool_c::Instance().Get_endpoint();

    // Define the callbacks for connection events
    metadata_c::on_callbacks_t callbacks = 
    {
        .on_open = open_callback,   // Set the open callback
        .on_close = close_callback, // Set the close callback
        .on_msg = on_msg_callback,  // Set the message callback
    };

    int id_connection; // Variable to hold the connection ID
    std::future<metadata_c::status_e> future; // Future to get the connection status

    // Attempt to create a new websocket connection
    auto err = endpoint.Create_new_connection("wss://echo.websocket.org",
                                              future,
                                              id_connection,
                                              callbacks);

    // Check for errors during connection creation
    if (err)
        throw std::runtime_error(std::string(err->Error_msg()));

    // Wait for the connection status and check if it is opened
    auto status = future.get();
    if (status != metadata_c::status_e::OPENED)
        throw std::runtime_error("Connection failed");

    std::cout << "Connection successful" << std::endl;

    // Send a message through the websocket
    err = endpoint.Send_msg(id_connection, "Hello, world!");
    if (err)
        throw std::runtime_error(std::string(err->Error_msg()));

    std::cout << "Message sent" << std::endl;

    // Wait for the echo response
    echo_got_future.wait();

    // Close the connection
    err = endpoint.Close(id_connection);
    if (err)
        throw std::runtime_error(std::string(err->Error_msg()));

    return 0; // Exit the program successfully
}