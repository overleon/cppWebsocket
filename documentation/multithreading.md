# Using the WebSocket Library: Multithreading Example

This document explains how to use the WebSocket library with multithreading. The example demonstrates how to create multiple WebSocket connections, each managed by a separate thread, showcasing the library's multithreading capabilities.

---

## 1. Getting Endpoint Instances

The library provides an **endpoint pool**, a singleton that manages WebSocket endpoints. You can retrieve endpoint instances to create connections. In this example, multiple threads access different endpoints to manage WebSocket connections concurrently:

```cpp
auto &pool = endpoint_pool_c::Instance();
size_t num_endpoints = pool.Max_index() + 1;

logger.Log("Number of endpoints in the pool: ", num_endpoints);
```

The `Max_index()` method provides the total number of available endpoints in the pool.

---

## 2. Defining the Connection Context

Each thread needs its own connection context to manage WebSocket events independently. The context holds information like the thread ID and promises for signaling message reception:

```cpp
struct ConnectionContext {
    std::promise<bool> echo_received;  // Promise to signal message reception
    std::future<bool> echo_future;    // Future associated with the promise
    int thread_id;                    // ID of the thread managing the connection

    ConnectionContext(int thread_id)
        : echo_future(echo_received.get_future()), thread_id(thread_id) {}
};
```

This ensures each thread can handle its own connection and responses without interference.

---

## 3. Handling WebSocket Events

Callback functions are defined to handle WebSocket events for each connection. For example:

- **Message Received Callback:**

```cpp
static void on_msg_callback(metadata_c::client &client,
                            websocketpp::connection_hdl conn_hdl,
                            metadata_c::client::message_ptr msg,
                            simdjson::ondemand::parser &parser,
                            ConnectionContext *context)
{
    if (msg->get_payload() == "Hello from endpoint " + std::to_string(context->thread_id)) {
        logger.Log("Message received on thread ", context->thread_id,
                                         ": ", msg->get_payload());
        context->echo_received.set_value(true);  // Signal that the message was received
    }
}
```

- **Other Callbacks:**

```cpp
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
```

These callbacks handle connection opening, closing, and message reception for each thread.

---

## 4. Creating WebSocket Connections in Threads

The `process_endpoint` function manages WebSocket connections for a single endpoint in a thread:

```cpp
void process_endpoint(endpoint_c &endpoint, int thread_id)
{
    auto context = std::make_shared<ConnectionContext>(thread_id);
    std::future<metadata_c::status_e> future;

    metadata_c::on_callbacks_t callbacks = { ... }; // Define your callbacks

    int id_connection;
    auto err = endpoint.Create_new_connection("wss://echo.websocket.org", future, id_connection, callbacks);

    if (err) {
        logger.Log("Error creating connection for endpoint ", thread_id, ": ", err->Error_msg());
        return;
    }

    if (future.get() != metadata_c::status_e::OPENED) {
        logger.Log("Connection failed for endpoint ", thread_id);
        return;
    }

    logger.Log("Connection successful for endpoint ", thread_id);

    std::string message = "Hello from endpoint " + std::to_string(thread_id);
    err = endpoint.Send_msg(id_connection, message);

    if (err) {
        logger.Log("Error sending message for endpoint ", thread_id, ": ", err->Error_msg());
        return;
    }

    logger.Log("Message sent from endpoint ", thread_id);
    context->echo_future.wait();
    endpoint.Close(id_connection);
}
```

This function handles all steps: creating a connection, sending a message, waiting for a response, and closing the connection.

---

## 5. Managing Threads

The `main` function spawns a thread for each endpoint to manage WebSocket connections concurrently:

```cpp
std::vector<std::thread> threads;
for (size_t i = 0; i < num_endpoints; ++i) {
    threads.emplace_back(process_endpoint, std::ref(pool.Get_endpoint(i)), i);
}

// Wait for all threads to complete
for (auto &thread : threads) {
    thread.join();
}
```

This ensures that multiple WebSocket connections are handled in parallel, leveraging the library's multithreading capabilities.

---

## Summary

This example demonstrates the following key features of the library:
1. Using an endpoint pool to retrieve endpoint instances.
2. Defining callbacks for handling WebSocket events.
3. Managing multiple WebSocket connections concurrently using threads.
4. Efficiently handling thread-specific contexts for independent WebSocket communication.

This approach makes the library suitable for scalable systems requiring multiple simultaneous WebSocket connections.
