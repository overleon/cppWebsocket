cmake_minimum_required(VERSION 3.10)
project(cppWebsocket VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Fetch simdjson library
include(FetchContent)
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(simdjson)

# Find necessary dependencies
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

if(NOT TARGET overHere_cppError)
    add_subdirectory(third-party/cppError overHere_cppError)
endif()

# websocketpp as header-only interface library
add_library(websocketpp INTERFACE)
target_include_directories(websocketpp INTERFACE third-party/websocketpp)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/cppWebsocket/inc
)

set(SOURCES
    cppWebsocket/src/ws/endpoint_pool.cpp
    cppWebsocket/src/ws/endpoint.cpp
    cppWebsocket/src/ws/metadata.cpp
    cppWebsocket/src/helpers/promise.cpp
)

# Create a separate library for the ws code and set its C++ standard to 17
add_library(cppWebsocket ${SOURCES})
target_include_directories(cppWebsocket
    PUBLIC ${HEADERS}
)
target_link_libraries(cppWebsocket PUBLIC 
    simdjson
    pthread
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    overHere_cppError
    websocketpp
)

if(BUILD_EXAMPLES_CPPWEBSOCKET)
    add_executable(simple examples/simple.cpp)
    target_link_libraries(simple PRIVATE cppWebsocket)

    add_executable(endpoint_pool examples/endpoint_pool.cpp)
    target_link_libraries(endpoint_pool PRIVATE cppWebsocket)

    add_executable(multithreading examples/multithreading.cpp)
    target_link_libraries(multithreading PRIVATE cppWebsocket)
endif()