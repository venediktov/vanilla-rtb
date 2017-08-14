/* 
 * File:   any_mapper.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on July 30, 2017, 10:01 PM
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
#ifndef RTB_DSL_ANY_MAPPER_HPP
#define RTB_DSL_ANY_MAPPER_HPP

#include <boost/any.hpp>
#include "extractors.hpp"
#include "rapid_serializer.hpp"
#include "rapidjson/writer.h"


namespace DSL {
        
    template<typename T>
    class any_mapper {

    protected:
        using encoded_type =  boost::any;
    public :
        using deserialized_type = openrtb::BidRequest<T>;
        using serialized_type = openrtb::BidResponse<T>;
        using parse_error_type = boost::bad_any_cast;
        
        using Impression = openrtb::Impression<T>;
        using Bid = openrtb::Bid<T>;
        using SeatBid = openrtb::SeatBid<T>;
        
        template<typename Deserialized, typename string_view_type>
        Deserialized extract(const string_view_type & bid_request) {
            jsmn_parser parser;
            jsmntok_t t[128];
            thread_local encoded_type encoded;
            clear(encoded);
            jsmn_init(&parser);
            auto r = jsmn_parse(&parser, bid_request.c_str(), bid_request.length(), t, sizeof(t)/sizeof(t[0]));
            if (r < 0) {
                throw std::runtime_error("DSL::jsmn_parse exception");
            }
            encoders::encode(bid_request.c_str(), &t[0], parser.toknext, encoded);
            return extractors<Deserialized>::extract(encoded);
        }

        template<typename Serialized>
        auto serialize(const Serialized &bid_response) {
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            serializer<Serialized>::template serialize(bid_response, writer);
            return std::string(sb.GetString(),sb.GetSize());
        }
        

        void clear(encoded_type &encoded) {
            boost::any tmp;
            boost::swap(encoded,tmp);
        }
    };
}


#endif /* RTB_DSL_ANY_MAPPER_HPP */

