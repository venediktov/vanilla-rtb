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
#include "dsl_mapper.hpp" //temporary dependency for reply formats

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
        
        template<typename Deserialized, typename Formats>
        Deserialized extract(boost::any & encoded, Formats &&) {
            return extractors<Deserialized>::extract(encoded);
        }
        
        auto build_request() {
            dsl_mapper<T> dm;
            return dm.build_request(); //TODO: return void* 
        }
        
        auto build_response() {
            dsl_mapper<T> dm;
            return dm.build_response();
        }
        
        void clear(encoded_type &encoded) {
            boost::any tmp;
            boost::swap(encoded,tmp);
        }
    };
}


#endif /* RTB_DSL_ANY_MAPPER_HPP */

