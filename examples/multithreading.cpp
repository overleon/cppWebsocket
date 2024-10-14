#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include "overHere/net/ws/ws/endpoint_pool.hpp"
#include "helpers/thread_logger.hpp"

using namespace overHere::net::ws;
auto &logger = ThreadSafeLog::Instance();

std::atomic<int> connections_completed(0);

struct ConnectionContext {
    std::promise<bool> echo_received;
    std::future<bool> echo_future;
    int thread_id;

    ConnectionContext(int thread_id)
        : echo_future(echo_received.get_future()), thread_id(thread_id) {}
};

static void on_msg_callback(metadata_c::client &client,
                            websocketpp::connection_hdl conn_hdl,
                            metadata_c::client::message_ptr msg,
                            simdjson::ondemand::parser &parser,
                            ConnectionContext *context)
{
    if (msg->get_payload() == "Hello from endpoint " + std::to_string(context->thread_id)) {
        logger.Log("Message received on thread ", context->thread_id,
                                         ": ", msg->get_payload());
        context->echo_received.set_value(true);
    }
}

void process_endpoint(endpoint_c &endpoint, int thread_id)
{
    auto context = std::make_shared<ConnectionContext>(thread_id);
    std::future<metadata_c::status_e> future;

    metadata_c::on_callbacks_t callbacks = {
        .on_open = [context](metadata_c::client &client, websocketpp::connection_hdl conn_hdl) {
            logger.Log("Connection opened on thread ", context->thread_id);
        },
        .on_close = [](uint16_t close_code, std::string_view msg) {
            logger.Log("Connection closed");
            connections_completed.fetch_add(1, std::memory_order_relaxed);
        },
        .on_msg = [context](metadata_c::client &client, websocketpp::connection_hdl hdl,
                            metadata_c::client::message_ptr msg, simdjson::ondemand::parser &parser) mutable 
        {
            on_msg_callback(client, hdl, msg, parser, context.get());
        }
    };

    int id_connection;
    auto err = endpoint.Create_new_connection("wss://echo.websocket.org", future, id_connection, callbacks);
    logger.Log("Connection created on thread ", thread_id);
    if (err) {
        logger.Log("Error creating connection for endpoint ", thread_id,
                                         ": ", err->Error_msg());
        return;
    }

    auto status = future.get();
    if (status != metadata_c::status_e::OPENED) {
        logger.Log("Connection failed for endpoint ", thread_id);
        return;
    }

    logger.Log("Connection successful for endpoint ", thread_id);

    std::string message = "Hello from endpoint " + std::to_string(thread_id);
    err = endpoint.Send_msg(id_connection, message);
    if (err) {
        logger.Log("Error sending message for endpoint ", thread_id,
                                         ": ", err->Error_msg());
        return;
    }

    logger.Log("Message sent from endpoint ", thread_id);

    context->echo_future.wait();

    err = endpoint.Close(id_connection);
    if (err) {
        logger.Log("Error closing connection for endpoint ", thread_id,
                                         ": ", err->Error_msg());
    }
}

int main()
{
    auto &pool = endpoint_pool_c::Instance();
    size_t num_endpoints = pool.Max_index() + 1;

    logger.Log("Number of endpoints in the pool: ", num_endpoints);

    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_endpoints; ++i) {
        threads.emplace_back(process_endpoint, std::ref(pool.Get_endpoint(i)), i);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    logger.Log("All connections completed: ", connections_completed.load());

    return 0;
}
