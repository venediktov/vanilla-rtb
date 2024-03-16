/* 
 * File:   dsl_mapper.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on June 5, 2017, 9:08 PM
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
#ifndef RTB_DSL_MAPPER_HPP
#define RTB_DSL_MAPPER_HPP

#include "jsonv/all.hpp"
#include "core/openrtb.hpp"
#include <vector>
#include <boost/optional.hpp>

namespace DSL {
    using namespace jsonv;

    template<typename T, unsigned int Size=128>
    class dsl_mapper {
    public:
        using deserialized_type = openrtb::BidRequest<T>;
        using serialized_type = openrtb::BidResponse<T>;
        using parse_error_type = jsonv::parse_error;
    private:
        //BidRequest
        using Banner = openrtb::Banner<T>;
        using AdPosition = openrtb::AdPosition;
        using Impression = openrtb::Impression<T>;
        using User = openrtb::User<T>;
        using Geo = openrtb::Geo<T>;
        using Site = openrtb::Site<T>;
        using Context = openrtb::Content<T>;
        using Publisher = openrtb::Publisher<T>;
        using BidRequest = openrtb::BidRequest<T>;
        //BidResponse
        using Bid = openrtb::Bid<T>;
        using SeatBid = openrtb::SeatBid<T>;
        using NoBidReason = openrtb::NoBidReason;
        using CreativeAttribute= openrtb::CreativeAttribute;
        using BidResponse = openrtb::BidResponse<T>;

    protected:
        using encoded_type =  jsonv::value;

        dsl_mapper() : request_fmt_{build_request()}, response_fmt_{build_response()}
        {}

        formats build_request() 
        {
            formats base_in = formats_builder()
                .type<Banner>()
                .member("h", &Banner::h)
                .member("w", &Banner::w)
                .member("pos", &Banner::pos)
                .template enum_type<AdPosition>("pos",
                {
                    { AdPosition::UNKNOWN,  0 },
                    { AdPosition::ABOVE, 1 },
                    { AdPosition::BETWEEN_DEPRECATED, 2 },
                    { AdPosition::BELOW, 3 },
                    { AdPosition::HEADER, 4 },
                    { AdPosition::FOOTER, 5 },
                    { AdPosition::SIDEBAR, 6 },
                    { AdPosition::FULLSCREEN,7 }
                })
                .template type<Impression>()
                .member("id", &Impression::id)
                .member("banner", &Impression::banner)
                .member("bidfloor", &Impression::bidfloor)
                .member("bidfloorcur", &Impression::bidfloorcur)
                .template type<User>()
                    .member("id", &User::id)
                    .member("buyeruid", &User::buyeruid)
                    .member("geo", &User::geo)
                .template type<Geo>()
                    .member("city", &Geo::city)
                    .member("country", &Geo::country)
                .template type<Site>()
                .template type<Context>()
                    .member("id", &Context::id)
                .template type<Publisher>()
                .template type<BidRequest>()
                .member("id", &BidRequest::id)
                .member("imp", &BidRequest::imp)
                .member("user", &BidRequest::user)
                .member("site", &BidRequest::site)
                .encode_if([](const jsonv::serialization_context&, const boost::optional<Site>& x) {return bool(x);})
                .template register_container<std::vector<Impression>>()
                .template register_optional<boost::optional<Banner>>()
                .template register_optional<boost::optional<Site>>()
                .template register_optional<boost::optional<Publisher>>()
                .template register_optional<boost::optional<User>>()
                .template register_optional<boost::optional<Geo>>()
                .template register_container<std::vector<T>>()
                .template register_container<std::vector<int>>()
                .check_references(formats::defaults())
                ;

                return formats::compose({ base_in, formats::defaults() });
         }

         formats build_response() 
         {
            formats base_out = formats_builder()
                .type<Bid>()
                .member("id", &Bid::id)
                .member("impid", &Bid::impid)
                .member("price", &Bid::price)
                .member("adid", &Bid::adid)
                .member("nurl", &Bid::nurl)
                .member("adm", &Bid::adm)
                .member("adomain", &Bid::adomain)
                .member("iurl", &Bid::iurl)
                .member("cid", &Bid::cid)
                .member("crid", &Bid::crid)
                .member("attr", &Bid::attr)
                .template enum_type<CreativeAttribute>("attr",
                {
                    { CreativeAttribute::UNDEFINED,  -1},
                    { CreativeAttribute::AUDIO_AD_AUTO_PLAY, 1},
                    { CreativeAttribute::AUDIO_AD_USER_INITIATED, 2},
                    { CreativeAttribute::EXPANDABLE_AUTOMATIC, 3},
                    { CreativeAttribute::EXPANDABLE_USER_INITIATED_CLICK, 4},
                    { CreativeAttribute::EXPANDABLE_USER_INITIATED_ROLLOVER, 5},
                    { CreativeAttribute::IN_BANNER_VIDEO_AD_AUTO_PLAY, 6},
                    { CreativeAttribute::IN_BANNER_VIDEO_AD_USER_INITIATED, 7},
                    { CreativeAttribute::POP, 8},
                    { CreativeAttribute::PROVOCATIVE_OR_SUGGESTIVE_IMAGERY, 9},
                    { CreativeAttribute::SHAKY_FLASHING_FLICKERING_EXTREME_ANIMATION_SMILEYS, 10},
                    { CreativeAttribute::SURVEYS, 11},
                    { CreativeAttribute::TEXT_ONLY, 12},
                    { CreativeAttribute::USER_INTERACTIVE, 13},
                    { CreativeAttribute::WINDOWS_DIALOG_OR_ALERT_STYLE, 14},
                    { CreativeAttribute::HAS_AUDIO_ON_OFF_BUTTON, 15},
                    { CreativeAttribute::AD_CAN_BE_SKIPPED, 16}
                })
                .template type<SeatBid>()
                .member("bid", &SeatBid::bid)
                .member("seat", &SeatBid::seat)
                .member("group", &SeatBid::group)
                .member("ext", &SeatBid::ext)
                .template type<BidResponse>()
                .member("id", &BidResponse::id)
                .member("seatbid", &BidResponse::seatbid)
                .member("bidid", &BidResponse::bidid)
                .member("cur", &BidResponse::cur)
                .member("customdata", &BidResponse::customdata)
                .member("nbr", &BidResponse::nbr)
                .member("ext", &BidResponse::ext)
                .template enum_type<NoBidReason>("nbr",
                {
                    { NoBidReason::UNKNOWN_ERROR,  0 },
                    { NoBidReason::TECHNICAL_ERROR, 1 },
                    { NoBidReason::INVALID_REQUEST, 2 },
                    { NoBidReason::KNOWN_WEB_SPIDER, 3 },
                    { NoBidReason::SUSPECTED_NON_HUMAN_TRAFFIC, 4 },
                    { NoBidReason::CLOUD_DATACENTER_OR_PROXY_IP, 5 },
                    { NoBidReason::UNSUPPORTED_DEVICE, 6 },
                    { NoBidReason::BLOCKED_PUBLISHER_OR_SITE,7 },
                    { NoBidReason::UNMATCHED_USER,8 }
                })
                .template register_container<std::vector<SeatBid>>()
                .template register_container<std::vector<Bid>>()
                .template register_container<std::vector<CreativeAttribute>>()
                .template register_container<std::vector<T>>()
                .check_references(formats::defaults())
                ;

            return formats::compose({ base_out, formats::defaults() });

        }

        template<typename Deserialized, typename string_view_type>
        Deserialized extract(const string_view_type &bid_request) {
            jsmn_parser parser;
            jsmntok_t t[Size];
            thread_local encoded_type encoded;
            clear(encoded);
            jsmn_init(&parser);
            auto r = jsmn_parse(&parser, bid_request.c_str(), bid_request.length(), t, sizeof(t)/sizeof(t[0]));
            if (r < 0) {
                throw std::runtime_error("DSL::jsmn_parse exception");
            }
            encoders::encode(bid_request.c_str(), &t[0], parser.toknext, encoded);
            return jsonv::extract<Deserialized>(encoded, request_fmt_);
        }
        template<typename Serialized>
        auto serialize(const Serialized &bid_response) {
            return to_string(to_json(bid_response, response_fmt_));
        }

        void clear(encoded_type &encoded) {
            encoded.clear();
        }
        formats request_fmt_;
        formats response_fmt_;
        
    };

} //namespace

#endif
