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
#include "selector.hpp"

namespace vanilla {
    template<typename Config = BidderConfig>
    class ResponseBuilder {
    public:
        ResponseBuilder(const BidderConfig &config) :
            selector{config}, uuid_generator{}
        {
        }

        openrtb::BidResponse build(const openrtb::BidRequest &request) {
            openrtb::BidResponse response;
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "fill response");
                for (auto &imp : request.imp) {

                    buildImpResponse(request, response, imp);
                }
            }
            LOG(debug) << sp->str();
            return response;
        }
    private:

        inline void addCurrency(const openrtb::BidRequest& request, openrtb::BidResponse& response, const openrtb::Impression& imp) {
            if (request.cur.size()) {
                response.cur = request.cur[0];
            } else if (imp.bidfloorcur.length()) {
                response.cur = imp.bidfloorcur; // Just return back
            }
        }

        inline void addBid(const openrtb::BidRequest& request, openrtb::BidResponse& response, const openrtb::Impression& imp, const std::shared_ptr<Ad> &ad) {
            if (response.seatbid.size() == 0) {
                response.seatbid.emplace_back(openrtb::SeatBid());
            }
            openrtb::Bid bid;
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

        inline void buildImpResponse(const openrtb::BidRequest& request, openrtb::BidResponse& response, const openrtb::Impression& imp) {
            if (auto ad = selector.getAd(request, imp)) {
                boost::uuids::uuid bidid = uuid_generator();
                response.bidid = boost::uuids::to_string(bidid);

                addCurrency(request, response, imp);
                addBid(request, response, imp, ad);
            }
        }
        vanilla::Selector<Config> selector;
        boost::uuids::random_generator uuid_generator;
    };
}
#endif /* RESPONSE_BULDER_HPP */

