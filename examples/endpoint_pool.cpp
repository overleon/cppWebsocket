#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include "overHere/net/ws/ws/endpoint_pool.hpp"

using namespace overHere::net::ws;

// Atomic counter for completed connections
std::atomic<int> connections_completed(0);

// Structure to hold context for each connection
struct ConnectionContext
{
    std::promise<bool> echo_received;
    std::future<bool> echo_future;
    int thread_id;

    ConnectionContext(int thread_id)
        : echo_future(echo_received.get_future()),
          thread_id(thread_id) {}
};

// Callback function for handling received messages
static void on_msg_callback(metadata_c::client &client,
                            websocketpp::connection_hdl conn_hdl,
                            metadata_c::client::message_ptr msg,
                            simdjson::ondemand::parser &parser,
                            ConnectionContext *context)
{
    if (msg->get_payload() == "Hello from endpoint " + std::to_string(context->thread_id))
    {
        std::cout << "Message received on thread " << context->thread_id
                  << ": " << msg->get_payload() << std::endl;
        context->echo_received.set_value(true);
    }
}

// Function to process each endpoint in a separate thread
void process_endpoint(endpoint_c &endpoint, int thread_id)
{
    // Create a shared pointer to the connection context
    auto context = std::make_shared<ConnectionContext>(thread_id);
    std::future<metadata_c::status_e> future;

    // Define callbacks for WebSocket events
    metadata_c::on_callbacks_t callbacks = {
        .on_open = [context](metadata_c::client &client, websocketpp::connection_hdl conn_hdl) {
            std::cout << "Connection opened on thread " << context->thread_id << std::endl;
        },
        .on_close = [](uint16_t close_code, std::string_view msg) {
            std::cout << "Connection closed" << std::endl;
            // Thread-safe increment of the completed connections counter
            connections_completed.fetch_add(1, std::memory_order_relaxed);
        },
        .on_msg = [context](
                      metadata_c::client &client,
                      websocketpp::connection_hdl hdl,
                      metadata_c::client::message_ptr msg,
                      simdjson::ondemand::parser &parser) mutable 
        {
            on_msg_callback(client, hdl, msg, parser, context.get());
        }
    };

    // Create a new WebSocket connection
    int id_connection;
    auto err = endpoint.Create_new_connection("wss://echo.websocket.org",
                                              future,
                                              id_connection,
                                              callbacks);
    std::cout << "Connection created on thread " << thread_id << std::endl;
    if (err)
    {
        std::cerr << "Error creating connection for endpoint " << thread_id
                  << ": " << err->Error_msg() << std::endl;
        return;
    }

    // Wait for the connection to be established
    auto status = future.get();
    if (status != metadata_c::status_e::OPENED)
    {
        std::cerr << "Connection failed for endpoint " << thread_id << std::endl;
        return;
    }

    std::cout << "Connection successful for endpoint " << thread_id << std::endl;

    // Send a test message
    std::string message = "Hello from endpoint " + std::to_string(thread_id);
    err = endpoint.Send_msg(id_connection, message);
    if (err)
    {
        std::cerr << "Error sending message for endpoint " << thread_id
                  << ": " << err->Error_msg() << std::endl;
        return;
    }

    std::cout << "Message sent from endpoint " << thread_id << std::endl;

    // Wait for the echo response
    // This is thread-safe since only this thread is accessing the future
    context->echo_future.wait();

    // Close the connection
    err = endpoint.Close(id_connection);
    if (err)
    {
        std::cerr << "Error closing connection for endpoint " << thread_id
                  << ": " << err->Error_msg() << std::endl;
    }
}

int main()
{
    // Get the endpoint pool instance
    auto &pool = endpoint_pool_c::Instance();
    size_t num_endpoints = pool.Max_index() + 1;

    std::cout << "Number of endpoints in the pool: " << num_endpoints << std::endl;

    // Create and start threads for each endpoint
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_endpoints; ++i)
    {
        threads.emplace_back(process_endpoint, std::ref(pool.Get_endpoint(i)), i);
    }

    // Wait for all threads to complete
    for (auto &thread : threads)
    {
        thread.join();
    }

    std::cout << "All connections completed: " << connections_completed << std::endl;

    return 0;
}
