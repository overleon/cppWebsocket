# cppWebsocket
overHere-cppWebsocket is a library thought to be used on single thread and multithreading systems in an easy way. The library has singleton of an endpoint pool, where **endpoint** represent a thread safety instance used to create and manage new websocket connections.  

---

## Examples folder
Under **Example** folder you will find two different examples for the usage of the library. Through these examples you will understand the usage of the library in an easy way.

---

## Dependencies
To use the library it is necessary to install the next libraries in your system:
* Boost
* OpenSSL

---

## Documentation
The [documentation](documentation/cppWebsocket.md) explains the usage of the library through the examples.

## Improvements
* Check whether it is possible to implement promises/futures for "Send_msg" method.
* Implement promises/futures for "Close" method.
