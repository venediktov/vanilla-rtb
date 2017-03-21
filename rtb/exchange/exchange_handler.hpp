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
#include <future>
#include <boost/asio.hpp>
#include "CRUD/service/reply.hpp"
#include "CRUD/handlers/crud_matcher.hpp"

namespace vanilla { namespace exchange {


template<typename  DSL>
class exchange_handler {

using auction_request_type = decltype(DSL().extract_request(std::string()));
using auction_response_type = typename DSL::serialized_type;
using parse_error_type = typename DSL::parse_error_type;
using auction_handler_type = std::function<auction_response_type (const auction_request_type &)>;
using auction_async_handler_type = std::function<void (const auction_request_type &, auction_response_type&)>;
using log_handler_type = std::function<void (const std::string &)>;
using error_log_handler_type = std::function<void (const std::string &)>;
using self_type = exchange_handler<DSL> ;

DSL parser;
auction_handler_type auction_handler;
auction_async_handler_type auction_async_handler;
log_handler_type log_handler;
error_log_handler_type error_log_handler;
const std::chrono::milliseconds tmax;
boost::asio::io_service io_service;
boost::asio::deadline_timer timer;

public:
    exchange_handler(const std::chrono::milliseconds &tmax) : 
        parser{}, auction_handler{}, log_handler{}, tmax{tmax}, io_service{}, timer{io_service}
    {}


    self_type & auction(const auction_handler_type &handler) {
        auction_handler = handler;
        return *this;
    }

    self_type & auction_async(const auction_async_handler_type &handler) {
        auction_async_handler = handler;
        return *this;
    }

    self_type & logger(const log_handler_type &handler) {
        log_handler = handler;
        return *this;
    }
    
    self_type & error_logger(const error_log_handler_type &handler) {
        error_log_handler = handler;
        return *this;
    }
    
    template<typename Match>
    void handle_post(http::server::reply & r, const http::crud::crud_match<Match>  & match) {
        if ( log_handler ) {
            log_handler(match.data);
        }
        auction_request_type bid_request;
        try {
            bid_request = parser.extract_request(match.data) ;
        } 
        catch (const parse_error_type &err) {
            if (error_log_handler) {
                error_log_handler(to_string(err));
            }
            r << http::server::reply::flush("");
            return;
        }

        if ( auction_handler ) {
            std::chrono::milliseconds timeout{bid_request.tmax ? bid_request.tmax : tmax.count()};
            auto future = std::async(std::launch::async, [&](){ 
                auto auction_response = auction_handler(bid_request) ;
                auto wire_response = parser.create_response(auction_response) ;
                return wire_response;
            });
            if ( future.wait_for(timeout) == std::future_status::ready) {
                r << to_string(future.get()) << http::server::reply::flush("");
            } else {
                r << http::server::reply::flush("");
            }
        }

        if ( auction_async_handler ) {
            std::chrono::milliseconds timeout{bid_request.tmax ? bid_request.tmax : tmax.count()};
            auction_response_type auction_response;
            auto submit_async = [&]() {
                auction_async_handler(bid_request,auction_response);
            };
            io_service.post(submit_async);
            timer.expires_from_now(boost::posix_time::milliseconds(timeout.count()));
            timer.async_wait( [this,&r,&auction_response](const boost::system::error_code& error ) {
               io_service.stop();
               if ( timer.expires_at() <= boost::asio::deadline_timer::traits_type::now() ) {
                  r << http::server::reply::flush("");
               } else {
                  auto wire_response = parser.create_response(auction_response);
                  r << to_string(wire_response) << http::server::reply::flush("");
               }
            });
            io_service.run();
            io_service.reset();
        }
    }
    
};

}}

