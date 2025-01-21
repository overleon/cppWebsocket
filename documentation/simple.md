# Using the WebSocket Library: A Simple Example

This document explains how to use the library through a simple example. The example demonstrates how to establish a WebSocket connection, send a message, and handle responses using the library.

---

## 1. Getting an Endpoint Instance

The library provides an **endpoint pool**, a singleton that manages WebSocket endpoints. You can retrieve a random endpoint instance from the pool using:

```cpp
auto &endpoint = endpoint_pool_c::Instance().Get_endpoint();
```

The `Get_endpoint()` method ensures you get a random endpoint instance, which can be used to create 'x' number of connections, managed by one single thread.

---

## 2. Defining Callback Functions

To handle WebSocket events, such as when the connection is opened, closed, or when a message is received, you can define callback functions:

```cpp
static void open_callback(metadata_c::client &client, websocketpp::connection_hdl conn_hdl)
{
    std::cout << "Connection opened" << std::endl;
}

static void close_callback(uint16_t close_code, std::string_view msg)
{
    std::cout << "Connection closed" << std::endl;
}

static void on_msg_callback(metadata_c::client &client, websocketpp::connection_hdl conn_hdl,
                            metadata_c::client::message_ptr msg, simdjson::ondemand::parser &parser)
{    
    if (msg->get_payload() == "Hello, world!")
    {
        std::cout << "Message received: " << msg->get_payload() << std::endl;
        echo_got.set_value(true); // Signal that the message has been received
    }
}
```

These callbacks provide a structured way to interact with WebSocket events.

---

## 3. Creating a Connection

Once you have an endpoint instance, you can use it to create a new WebSocket connection. You'll need to pass the event callbacks:

```cpp
metadata_c::on_callbacks_t callbacks = {
    .on_open = open_callback,
    .on_close = close_callback,
    .on_msg = on_msg_callback,
};

int id_connection;
std::future<metadata_c::status_e> future;

auto err = endpoint.Create_new_connection("wss://echo.websocket.org", future, id_connection, callbacks);

if (err)
    throw std::runtime_error(std::string(err->Error_msg()));
```

This creates a connection to `wss://echo.websocket.org`. The `future` object is used to track the connection status.

---

## 4. Verifying the Connection

After creating the connection, you can verify if it was successfully established:

```cpp
auto status = future.get();
if (status != metadata_c::status_e::OPENED)
    throw std::runtime_error("Connection failed");

std::cout << "Connection successful" << std::endl;
```

---

## 5. Sending a Message

Once the connection is open, you can send a message through the WebSocket:

```cpp
err = endpoint.Send_msg(id_connection, "Hello, world!");
if (err)
    throw std::runtime_error(std::string(err->Error_msg()));

std::cout << "Message sent" << std::endl;
```

---

## 6. Waiting for a Response

The example waits for an echo response from the server:

```cpp
echo_got_future.wait();
```

This ensures that the program does not proceed until the response is received.

---

## 7. Closing the Connection

Finally, close the WebSocket connection:

```cpp
err = endpoint.Close(id_connection);
if (err)
    throw std::runtime_error(std::string(err->Error_msg()));
```

---

## Summary

This example highlights the following key steps for using the library:
1. Retrieve an endpoint instance from the endpoint pool.
2. Define callback functions for handling WebSocket events.
3. Create a new WebSocket connection with the desired callbacks.
4. Verify the connection status.
5. Send messages and handle responses.
6. Close the connection when done.

This approach provides a simple and efficient way to work with WebSocket connections using the library.