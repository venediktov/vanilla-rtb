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
#include <boost/optional.hpp>
#include "CRUD/service/reply.hpp"
#include "CRUD/handlers/crud_matcher.hpp"
#include <rtb/common/decision_tree.hpp>

namespace vanilla {
    namespace exchange {

        thread_local boost::asio::io_service io_service;
        thread_local boost::asio::deadline_timer timer{io_service};


        template<typename DSL>
        class exchange_handler {
            using auction_request_type = decltype(DSL().extract_request(std::string()));
            using auction_response_type = typename DSL::serialized_type;
            using wire_response_type = decltype(DSL().create_response(auction_response_type()));
            using parse_error_type = typename DSL::parse_error_type;
            using auction_handler_type = std::function<auction_response_type(const auction_request_type &)>;
            using auction_async_handler_type = auction_handler_type;
            using log_handler_type = std::function<void (const std::string &)>;
            using error_log_handler_type = std::function<void (const std::string &)>;
            using self_type = exchange_handler<DSL>;
        public:
            using decision_params_type = std::tuple<self_type&, http::server::reply&, auction_request_type &>;
        private:
            using decision_handler_type = std::function<void (decision_params_type &&)>;
            
            DSL parser;
            auction_handler_type auction_handler;
            auction_async_handler_type auction_async_handler;
            log_handler_type log_handler;
            error_log_handler_type error_log_handler;
            decision_handler_type decision_handler;
            
            const std::chrono::milliseconds tmax;

        public:
            
            exchange_handler(const std::chrono::milliseconds &tmax) :
                parser{}, auction_handler{}, log_handler{}, tmax{tmax}
            {
            }

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
            
            self_type & decision(const decision_handler_type &handler) {
                decision_handler = handler;
                return *this;
            }

            bool handle_auction(http::server::reply& r, auction_request_type bid_request) {
                if (auction_handler) {
                    std::chrono::milliseconds timeout{bid_request.tmax ? bid_request.tmax : tmax.count()};
                    auto future = std::async(std::launch::async, [&]() {
                        auto auction_response = auction_handler(bid_request);
                        auto wire_response = parser.create_response(auction_response);
                        return wire_response;
                    });
                    if (future.wait_for(timeout) == std::future_status::ready) {
                        r << to_string(future.get()) << http::server::reply::flush("");
                    } else {
                        r << http::server::reply::flush("");
                    }
                    return true;
                }
                return false;
            }

            bool hanle_auction_async(http::server::reply& r, auction_request_type bid_request) {
                if (auction_async_handler) {
                    std::chrono::milliseconds timeout{bid_request.tmax ? bid_request.tmax : tmax.count()};
                    boost::optional<wire_response_type> wire_response;
                    auto submit_async = [&]() {
                        auto auction_response = auction_async_handler(bid_request);
                        wire_response = parser.create_response(auction_response);
                        io_service.stop();
                    };
                    io_service.post(submit_async);
                    timer.expires_from_now(boost::posix_time::milliseconds(timeout.count()));
                    timer.async_wait([](const boost::system::error_code & error) {
                        if (error != boost::asio::error::operation_aborted) {
                            io_service.stop();
                        }
                    });
                    io_service.reset();
                    io_service.run();
                    if (wire_response && timer.expires_from_now().total_milliseconds() > 0) {
                        r << to_string(*wire_response) << http::server::reply::flush("");
                    } else {
                        r << http::server::reply::flush("");
                    }
                    return true;
                }
                return false;
            }
        private:

            template<typename Match>
            bool handle_post_common(http::server::reply & r, const http::crud::crud_match<Match> &match, auction_request_type &bid_request) {
                if (log_handler) {
                    log_handler(match.data);
                }
                try {
                    bid_request = parser.extract_request(match.data);
                } catch (const parse_error_type &err) {
                    if (error_log_handler) {
                        error_log_handler(to_string(err));
                    }
                    r << http::server::reply::flush("");
                    return false;
                }
                return true;
            }
        public:

            template<typename Match>
            void handle_post(http::server::reply & r, const http::crud::crud_match<Match> & match) {
                auction_request_type bid_request;
                if (!handle_post_common(r, match, bid_request)) {
                    return;
                }

                if(decision_handler) {
                    decision_handler(std::tie(*this, r, bid_request));
                } else if (hanle_auction_async(r, bid_request)) {
                    ;
                }
                else if (handle_auction(r, bid_request)) {
                    ;
                }
            }
        };

    }
}

