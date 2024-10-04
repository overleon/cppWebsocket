#pragma once
#include <mutex>
#include <memory>

#include "overHere/net/ws/ws/endpoint.hpp"

namespace overHere::net::ws
{
    class endpoint_pool_c
    {
        private:
            std::mutex _mtx;
            std::vector<endpoint_c> _pool;
            size_t _iterator;
            size_t _max_iterator;
            
            endpoint_pool_c(const endpoint_c&) = delete;
            endpoint_c& operator=(const endpoint_c&) = delete;
            endpoint_pool_c();
            
        public:
            ~endpoint_pool_c();
            static endpoint_pool_c& Instance();
            endpoint_c &Get_endpoint(void);
            endpoint_c &Get_endpoint(uint8_t);
            size_t Max_index() const;
    };
}
