/*
 * File:   exchange_server.hpp
 * Author: Vladimir Venediktov
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on January 09, 2016, 12:40 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*
*/

#pragma once

#include <string>
#include <thread>
#include <tuple>
#include <functional>
#include <boost/asio.hpp>
#include "CRUD/service/server.hpp"

namespace vanilla { namespace exchange {

struct connection_endpoint {
    using ctor_type = std::tuple<std::string, std::string, std::string>;
    std::string host;
    std::string port;
    std::string root;
    connection_endpoint(const ctor_type &ep) : host{ std::get<0>(ep) }, port{ std::get<1>(ep) }, root{ std::get<2>(ep) }
    {}
   
};

//allows to implement custom dispatcher 
template<typename RestfulDispatcherT>
struct exchange_server {

connection_endpoint ep;
RestfulDispatcherT dispatcher;
unsigned int hardware_threads;

public:
    //make it non-copyable non-movable
    exchange_server(const exchange_server&) = delete;
    exchange_server& operator=(const exchange_server&) = delete;
    exchange_server(exchange_server&&) = delete;
    exchange_server& operator=(exchange_server&&) = delete;

    exchange_server(const connection_endpoint &ep, const RestfulDispatcherT &dispatcher) : 
        ep{ep}, dispatcher{dispatcher}, hardware_threads{std::max(1u, std::thread::hardware_concurrency()) }
    {}

    exchange_server& set_concurrency(unsigned int concurrency) {
        if(0 < concurrency) {
            hardware_threads = concurrency;
        }
        return *this;
    }
    void run() {
       http::server::server<RestfulDispatcherT> server{ep.host,ep.port,dispatcher} ;
       std::vector<std::shared_ptr<std::thread>> threads;
       for ( unsigned int i=0; i < hardware_threads ; ++i) {
            threads.push_back( std::make_shared<std::thread>( [&server] () {
                server.run() ;
            }));
       }

       for ( auto thread : threads ) {
           thread->join();
       }
    
    }

};

}}

