/* 
 * File:   response_builder.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 5 марта 2017 г., 22:25
 */

#pragma once 
#ifndef VANILLA_BIDDER_HPP
#define VANILLA_BIDDER_HPP

#include <memory>
#include <iostream>
#include "ad_selector.hpp"
#include "examples/multiexchange/user_info.hpp"

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
    
    template<typename DSL, typename Config = BidderConfig>
    class Bidder {
        using BidRequest  = typename DSL::deserialized_type;
        using BidResponse = typename DSL::serialized_type;
        using Impression  = typename DSL::Impression;
        using SeatBid     = typename DSL::SeatBid;
        using Bid         = typename DSL::Bid;

    public:
        Bidder(BidderCaches<Config> &caches) :
            selector{caches}, uuid_generator{}
        {}
        template <typename Request , typename ...Info>
        const BidResponse& bid(const Request &vanilla_request, Info && ...) {
            response.clear();
            auto request = request_extractor<Request>::request(vanilla_request);
            for (auto &imp : request.imp) {
                buildImpResponse(request, imp);
            }
            return response;
        }
    private:
        
        inline void addCurrency(const BidRequest& request, const Impression& imp) {
            if (request.cur.size()) {
                response.cur = request.cur[0];
            } else if (imp.bidfloorcur.length()) {
                response.cur = imp.bidfloorcur; // Just return back
            }
        }

        inline void addBid(const BidRequest& request, const Impression& imp, const std::shared_ptr<Ad> &ad) {
            if (response.seatbid.size() == 0) {
                response.seatbid.emplace_back(SeatBid());
            }
            Bid bid;
            boost::uuids::uuid bidid = uuid_generator();
            bid.id = boost::uuids::to_string(bidid); // TODO check documentation 
            // Is it the same as response.bidid?
            bid.impid = imp.id;
            bid.price = ad->max_bid_micros / 1000000.0; // Not micros?
            bid.w = ad->width;
            bid.h = ad->height;
            bid.adm = ad->code;
            bid.adid = ad->ad_id;
            response.seatbid.back().bid.emplace_back(std::move(bid));
        }

        inline void buildImpResponse(const BidRequest& request, const Impression& imp) {
            if (auto ad = selector.select(request, imp)) {
                boost::uuids::uuid bidid = uuid_generator();
                response.bidid = boost::uuids::to_string(bidid);

                addCurrency(request, imp);
                addBid(request, imp, ad);
            }
        }
        vanilla::AdSelector<Config> selector;
        boost::uuids::random_generator uuid_generator;
        BidResponse response;
    };
}
#endif /* VANILLA_BIDDER_HPP */

