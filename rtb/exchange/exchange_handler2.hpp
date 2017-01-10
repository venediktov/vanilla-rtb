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
#include <functional>
#include <chrono>
#include "CRUD/service/reply.hpp"
#include "CRUD/handlers/crud_matcher.hpp"

namespace vanilla { namespace exchange {


template<typename  DSL>
class exchange_handler {

using auction_request_type = decltype(DSL().extract_request(std::string()));
using auction_response_type = typename DSL::serialized_type;
using auction_handler_type = std::function<auction_response_type (const auction_request_type &,const std::chrono::milliseconds &)>; 
using log_handler_type = std::function<void (const std::string &)>;
using self_type = exchange_handler<DSL> ;

DSL parser;
auction_handler_type auction_handler;
log_handler_type log_handler;

public:
    exchange_handler() : parser{}, auction_handler{}, log_handler{} 
    {}

    self_type & auction(const auction_handler_type &handler) {
        auction_handler = handler;
        return *this;
    }

    self_type & logger(const log_handler_type &handler) {
        log_handler = handler;
        return *this;
    }

    template<typename Match>
    handle_post(http::server::reply & r, const http::crud::crud_match<Match>  & match) {
        if ( log_handler ) {
            log_handler(match.data);
        }
        auto bid_request = parser.extract_request(match.data) ;
        if ( auction_handler ) {
            std::chrono::milliseconds timeout{bid_request.tmax};
            auto auction_response = auction_handler(bid_request,timeout) ;
            auto wire_response = parser.create_response(auction_response) ;
            r << to_string(wire_response) << http::server::reply::flush("");
        }
    }
    
};

}}

