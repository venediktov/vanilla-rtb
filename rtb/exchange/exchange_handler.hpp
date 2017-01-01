/*
 * File:   exchange_handler.hpp
 * Author: Vladimir Venediktov
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on December 25, 2016, 12:40 PM
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
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include "CRUD/service/server.hpp"
#include "CRUD/service/request_handler.hpp"
#include "CRUD/service/request.hpp"
#include "CRUD/service/reply.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"

namespace vanilla { namespace exchange {

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

struct connection_endpoint {
    using ctor_type = std::tuple<std::string, std::string, std::string>;
    std::string host;
    std::string port;
    std::string root;
    connection_endpoint(const ctor_type &ep) : host{ std::get<0>(ep) }, port{ std::get<1>(ep) }, root{ std::get<2>(ep) }
    {}
   
};

template<typename  DSL>
class exchange_handler {

using restful_dispatcher_t  = http::crud::crud_dispatcher<http::server::request, http::server::reply> ;

restful_dispatcher_t handler_ ;
http::server::server<restful_dispatcher_t> server_;
unsigned int hardware_threads_;


    //make it non-copyable non-movable
    exchange_handler(const exchange_handler&) = delete;
    exchange_handler& operator=(const exchange_handler&) = delete;
    exchange_handler(exchange_handler&&) = delete;
    exchange_handler& operator=(exchange_handler&&) = delete;

public:
    exchange_handler(const connection_endpoint &ep, const boost::regex &expr) : 
       handler_{ep.root}, server_{ep.host,ep.port,handler_}, hardware_threads_{ std::max(1u, std::thread::hardware_concurrency()) }
    {
        handler_
            .crud_match(expr)
            .post([](http::server::reply & r, const http::crud::crud_match<boost::cmatch>  & match) {
            LOG(debug) << "POST request=" << match[0] ;
            LOG(debug) << "POST request_data=" << match.data ;
            //TODO: implement passing match.data to auction

        });

    }

    void run() {
       std::vector<std::shared_ptr<std::thread>> threads;
       for ( unsigned int i=0; i < std::thread::hardware_concurrency() ; ++i) {
            threads.push_back( std::make_shared<std::thread>( [this] () {
                server_.run() ;
            }));
       }

       for ( auto thread : threads ) {
           thread->join();
       }
    
    }

};

}}

