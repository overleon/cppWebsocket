#include "overHere/net/ws/ws/endpoint_pool.hpp"

#include <unistd.h>

namespace overHere::net::ws
{

    endpoint_pool_c::endpoint_pool_c()
        : _iterator(0), _pool(sysconf(_SC_NPROCESSORS_ONLN))
    {
        _max_iterator = _pool.size();
    }

    endpoint_pool_c::~endpoint_pool_c()
    {
        for(auto &endpoint : _pool)
        {
            auto err = endpoint.Close_all_connections();
            if(err)
                std::cout<<err->Error_msg()<<std::endl;
        }
        for(auto &endpoint : _pool) 
        {
            auto err = endpoint.Wait_for_closing_all();
            if(err)
                std::cout<<err->Error_msg()<<std::endl;
        }     
    }

    endpoint_pool_c& endpoint_pool_c::Instance()
    {
        static endpoint_pool_c instance;
        return instance;
    }

    endpoint_c &endpoint_pool_c::Get_endpoint(void)
    {
        std::lock_guard guard(_mtx);
        if(++_iterator >= _max_iterator)
            _iterator = 0;
        return _pool[_iterator];
    }

    endpoint_c &endpoint_pool_c::Get_endpoint(uint8_t idx)
    {
        std::lock_guard guard(_mtx);
        if(idx >= _max_iterator)
            return _pool[_max_iterator - 1];
        return _pool[idx];
    }

    size_t endpoint_pool_c::Max_index() const
    {
        return _max_iterator - 1;
    }
}