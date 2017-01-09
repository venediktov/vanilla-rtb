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
#include <functional>
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

using restful_dispatcher_type  = http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
using auction_request_type = decltype(DSL().extract_request(std::string()));
using auction_response_type = typename DSL::serialized_type;
using auction_handler_type = std::function<auction_response_type (const auction_request_type &,const std::chrono::milliseconds &)>; 
using log_handler_type = std::function<void (const http::crud::crud_match<boost::cmatch> &)>;
using self_type = exchange_handler<DSL> ;

connection_endpoint ep;
boost::regex expr;
unsigned int hardware_threads;
DSL parser;
auction_handler_type auction_handler;
log_handler_type log_handler;

public:
    //make it non-copyable non-movable
    exchange_handler(const exchange_handler&) = delete;
    exchange_handler& operator=(const exchange_handler&) = delete;
    exchange_handler(exchange_handler&&) = delete;
    exchange_handler& operator=(exchange_handler&&) = delete;

    exchange_handler(const connection_endpoint &ep, const boost::regex &expr) : 
        ep{ep}, expr{expr}, hardware_threads{ std::max(1u, std::thread::hardware_concurrency()) }
    {}

    self_type & auction(const auction_handler_type &handler) {
        auction_handler = handler;
        return *this;
    }

    self_type & logger(const log_handler_type &handler) {
        log_handler = handler;
        return *this;
    }
    
    void run() {
       restful_dispatcher_type handler{ep.root} ;
       handler.crud_match(expr)
              .post([this](http::server::reply & r, const http::crud::crud_match<boost::cmatch>  & match) {
                  if ( log_handler ) {
                      log_handler(match);
                  }
                  auto bid_request = parser.extract_request(match.data) ;
                  if ( auction_handler ) {
                      std::chrono::milliseconds timeout{bid_request.tmax};
                      auto auction_response = auction_handler(bid_request,timeout) ;
                      auto wire_response = parser.create_response(auction_response) ;
                      r << to_string(wire_response) << http::server::reply::flush("");
                  }
       });
       http::server::server<restful_dispatcher_type> server{ep.host,ep.port,handler} ;
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

