/* 
 * File:   rapid_mapper.hpp
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
#ifndef RTB_DSL_RAPID_MAPPER_HPP
#define RTB_DSL_RAPID_MAPPER_HPP

#include "extractors.hpp"
#include "rapid_serializer.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

namespace DSL {
        
    template<typename T>
    class rapid_mapper {
    protected:
        using encoded_type =  rapidjson::Document;
    public :
        using deserialized_type = openrtb::BidRequest<T>;
        using serialized_type = openrtb::BidResponse<T>;
        using parse_error_type = std::exception;
        
        using Impression = openrtb::Impression<T>;
        using Bid = openrtb::Bid<T>;
        using SeatBid = openrtb::SeatBid<T>;

        template<typename Deserialized, typename string_view_type>
        Deserialized extract(const string_view_type &bid_request) {
            thread_local encoded_type encoded;
            clear(encoded);
            encoded.Parse(bid_request.c_str());
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
            encoded_type tmp;
            std::swap(encoded,tmp);
        }
    };
}


#endif /* RTB_DSL_RAPID_MAPPER_HPP */

