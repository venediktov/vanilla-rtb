/* 
 * File:   response_builder.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 5 марта 2017 г., 22:25
 */

#ifndef RESPONSE_BULDER_HPP
#define RESPONSE_BULDER_HPP

#include <memory>
#include <iostream>
#include "rtb/common/perf_timer.hpp"
#include "bidder_selector.hpp"
#include "examples/multiexchange/user_info.hpp"

namespace vanilla {
    template<typename Config = BidderConfig, typename T = std::string>
    class Bidder {
        using BidRequest  = openrtb::BidRequest<T>;
        using BidResponse = openrtb::BidResponse<T>;
        using Impression  = openrtb::Impression<T>;
        using SeatBid     = openrtb::SeatBid<T>;
        using Bid         = openrtb::Bid<T>;

    public:
        Bidder(BidderCaches<> &caches) :
            selector{caches}, uuid_generator{}
        {
        }

        const BidResponse& build(const vanilla::VanillaRequest &vanilla_request) {
            response.clear();
            const BidRequest &request = vanilla_request.bid_request;
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
        vanilla::BidderSelector<Config> selector;
        boost::uuids::random_generator uuid_generator;
        BidResponse response;
    };
}
#endif /* RESPONSE_BULDER_HPP */

