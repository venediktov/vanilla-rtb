/*
 * File:   bidder.hpp
 * Author: Vladimir Venediktov
 * Author: Arseny Bushev
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on January 8, 2018, 2:42 PM
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
 */


#pragma once
#ifndef VANILLA_BIDDER_HPP
#define VANILLA_BIDDER_HPP

#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "user_info.hpp"

namespace vanilla {

    template <typename Request>
    struct request_extractor {
        static const Request& request(const Request &request) {
            return request;
        }
    };

    template <>
    struct request_extractor<VanillaRequest> {
        static auto request(const VanillaRequest &r) -> decltype(r.request()) {
            return r.request();
        }
    };

    template<typename DSL, typename Selector>
    class Bidder {
        using BidRequest  = typename DSL::deserialized_type;
        using BidResponse = typename DSL::serialized_type;

    public:
        Bidder(Selector selector) : selector{std::move(selector)}, uuid_generator{}
        {}
        template <typename Request , typename Tuple, std::size_t... Idx, typename ...Info>
        const BidResponse& bid(const Request &vanilla_request, Tuple&&  tuple, std::index_sequence<Idx...>, Info && ...) {
            response.clear();
            auto request = request_extractor<Request>::request(vanilla_request);
            for (auto &imp : request.imp) {
                buildImpResponse(request, imp, std::get<Idx>(std::forward<Tuple>(tuple))...);
            }
            return response;
        }
        template <typename Request , typename ...Keys>
        const BidResponse& bid(const Request &vanilla_request, Keys&&... keys) {
            return  this->bid(vanilla_request, std::make_tuple(std::forward<Keys>(keys)...),
                                               std::make_index_sequence<sizeof...(keys)>()
            );
        }
    private:

        template<typename Impression>
        void addCurrency(const BidRequest& request, const Impression& imp) {
            if (request.cur.size()) {
                response.cur = request.cur[0];
            } else if (imp.bidfloorcur.length()) {
                response.cur = imp.bidfloorcur; // Just return back
            }
        }

        template<typename Impression, typename Ad>
        void addBid(const BidRequest& request, const Impression& imp, Ad && ad) {
            if (response.seatbid.size() == 0) {
                response.seatbid.emplace_back();
            }
            typename std::remove_reference<decltype(BidResponse().seatbid[0].bid[0])>::type bid;
            boost::uuids::uuid bidid = uuid_generator();
            bid.id = boost::uuids::to_string(bidid); // TODO check documentation
            // Is it the same as response.bidid?
            bid.impid = imp.id;
            bid.price = ad.auth_bid_micros ? *ad.auth_bid_micros / 1000000.0 : ad.max_bid_micros / 1000000.0 ; // Not micros?
            bid.w = ad.width;
            bid.h = ad.height;
            bid.adm = ad.code;
            bid.adid = std::to_string(ad.ad_id); //for T = std::string , for T = string_view must use thread_local storage
            response.seatbid.back().bid.emplace_back(std::move(bid));
        }

        template<typename Impression, typename Arg, typename ...ChainedFuncs>
        void buildImpResponse(const BidRequest& request, const Impression& imp, Arg && arg, ChainedFuncs... chained) {
            if (auto ad = selector.select(request, imp, std::forward<Arg>(arg), chained... )) {
                boost::uuids::uuid bidid = uuid_generator();
                response.bidid = boost::uuids::to_string(bidid);

                addCurrency(request, imp);
                addBid(request, imp, *ad);
            }
        }
        Selector selector;
        boost::uuids::random_generator uuid_generator;
        BidResponse response;
    };
}
#endif /* VANILLA_BIDDER_HPP */


