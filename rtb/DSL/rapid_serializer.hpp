/* 
 * File:   rapid_serializer.hpp
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
#ifndef RTB_DSL_RAPID_SERIALIZER_HPP
#define RTB_DSL_RAPID_SERIALIZER_HPP

#include "rtb/core/openrtb.hpp"

namespace DSL {
        
template<typename T>
struct serializer {
template<typename Writer>
static void serialize( const T & response, Writer &writer );
template<typename Writer>
static void serialize( const char * data, const std::size_t size, Writer &writer ) {
    writer.String(data,size);
}
};

template<typename T>
struct serializer<std::vector<T>> {
    template<typename Writer>
    static void serialize( const std::vector<T> & values, Writer &writer ) {
        if ( values.empty() ) {
            writer.StartArray();
            writer.EndArray();
            return ;
        }
        writer.StartArray();
        for ( auto &value : values ) {
            serializer<T>::serialize(value,writer);
        }
        writer.EndArray();
    }
};

//struct declarations parcial specializations
template<typename T>
struct serializer<openrtb::BidResponse<T>> {
template<typename Writer>
static void serialize(const openrtb::BidResponse<T> &bid_response, Writer &writer) {
    writer.StartObject();
    if(bid_response.id.size()>0) {
        writer.String("id");
        serializer<decltype(bid_response.id)>::serialize(bid_response.id.data(), bid_response.id.size(), writer);
    }
    if(bid_response.seatbid.size()>0) {
        writer.String("seatbid");
        serializer<decltype(bid_response.seatbid)>::serialize(bid_response.seatbid, writer);
    }
    if(bid_response.bidid.size()>0) {
        writer.String("bidid");
        serializer<decltype(bid_response.bidid)>::serialize(bid_response.bidid.data(), bid_response.bidid.size(),
                                                            writer);
    }
    if(bid_response.cur.size()>0) {
        writer.String("cur");
        serializer<decltype(bid_response.cur)>::serialize(bid_response.cur.data(), bid_response.cur.size(), writer);
    }
    if(bid_response.customdata.size()>0) {
        writer.String("customdata");
        serializer<decltype(bid_response.customdata)>::serialize(bid_response.customdata.data(),
                                                                 bid_response.customdata.size(), writer);
    }
    writer.String("nbr")       ; serializer<decltype(bid_response.nbr)>::serialize(bid_response.nbr, writer);
    writer.EndObject();
}
};

template<typename T>
struct serializer<openrtb::SeatBid<T>> {
template<typename Writer>
static void serialize(const openrtb::SeatBid<T> &seatbid, Writer &writer) {
    writer.StartObject();
    writer.String("bid")  ; serializer<decltype(seatbid.bid)>::serialize(seatbid.bid, writer);
    if(seatbid.seat.size()>0) {
        writer.String("seat");
        serializer<decltype(seatbid.seat)>::serialize(seatbid.seat.data(), seatbid.seat.size(), writer);
    }
    if(seatbid.group>0) {
        writer.String("group");
        serializer<decltype(seatbid.group)>::serialize(seatbid.group, writer);
    }
    writer.EndObject();
}
};

template<>
struct serializer<openrtb::NoBidReason> {
template<typename Writer>
static void serialize(const openrtb::NoBidReason &nbr, Writer &writer) {
    writer.Int((int)nbr);
}
};
template<>
struct serializer<openrtb::CreativeAttribute> {
template<typename Writer>
static void serialize(const openrtb::CreativeAttribute &attr, Writer &writer) {
    writer.Int((int)attr);
}
};

template<>
struct serializer<int8_t> {
template<typename Writer>
static void serialize(const int8_t value, Writer &writer) {
    writer.Int(value);
}
};

template<>
struct serializer<int> {
template<typename Writer>
static void serialize(const int value, Writer &writer) {
    writer.Int(value);
}
};

template<>
struct serializer<double> {
template<typename Writer>
static void serialize(const double value, Writer &writer) {
    writer.Double(value);
}
};

template<typename T>
struct serializer<openrtb::Bid<T>> {
template<typename Writer>
static void serialize(const openrtb::Bid<T> &bid, Writer &writer) {
    writer.StartObject();
    writer.String("id")     ;  serializer<decltype(bid.id)>::serialize(bid.id.data(),bid.id.size(),writer);
    writer.String("impid")  ;  serializer<decltype(bid.impid)>::serialize(bid.impid.data(),bid.impid.size(),writer);
    writer.String("price")  ;  serializer<decltype(bid.price)>::serialize(bid.price, writer);
    if(bid.adid.size()>0) {
        writer.String("adid");
        serializer<decltype(bid.adid)>::serialize(bid.adid.data(), bid.adid.size(), writer);
    }
    if(bid.nurl.size()>0) {
        writer.String("nurl");
        serializer<decltype(bid.nurl)>::serialize(bid.nurl.data(), bid.nurl.size(), writer);
    }
    if(bid.burl.size()>0) {
        writer.String("burl");
        serializer<decltype(bid.nurl)>::serialize(bid.burl.data(), bid.burl.size(), writer);
    }
    if(bid.lurl.size()>0) {
        writer.String("lurl");
        serializer<decltype(bid.nurl)>::serialize(bid.lurl.data(), bid.lurl.size(), writer);
    }
    writer.String("adm")    ;  serializer<decltype(bid.adm)>::serialize(bid.adm.data(),bid.adm.size(),writer);
    if(bid.adomain.size()>0) {
        writer.String("adomain");
        writer.StartArray();
        for (auto &value: bid.adomain) {
            serializer<decltype(value)>::serialize(value.data(), value.size(), writer);
        }
        writer.EndArray();
    }
    if(bid.bundle.size()>0) {
        writer.String("bundle");
        serializer<decltype(bid.iurl)>::serialize(bid.bundle.data(), bid.bundle.size(), writer);
    }
    if(bid.iurl.size()>0) {
        writer.String("iurl");
        serializer<decltype(bid.iurl)>::serialize(bid.iurl.data(), bid.iurl.size(), writer);
    }
    if(bid.cid.size()>0) {
        writer.String("cid");
        serializer<decltype(bid.cid)>::serialize(bid.cid.data(), bid.cid.size(), writer);
    }
    if(bid.crid.size()>0) {
        writer.String("crid");
        serializer<decltype(bid.crid)>::serialize(bid.crid.data(), bid.crid.size(), writer);
    }
    if(bid.tactic.size()>0) {
        writer.String("tactic");
        serializer<decltype(bid.tactic)>::serialize(bid.tactic.data(), bid.tactic.size(), writer);
    }
    if(bid.cat.size()) {
        writer.String("cat");
        writer.StartArray();
        for (auto &value: bid.cat) {
            serializer<decltype(value)>::serialize(value.data(), value.size(), writer);
        }
        writer.EndArray();
    }
    if(bid.attr.size()>0) {
        writer.String("attr");
        serializer<decltype(bid.attr)>::serialize(bid.attr, writer);
    }
    if(bid.api>0) {
        writer.String("api");
        serializer<decltype(bid.api)>::serialize(bid.api, writer);
    }
    if(bid.protocol>0) {
        writer.String("protocol");
        serializer<decltype(bid.protocol)>::serialize(bid.protocol, writer);
    }
    if(bid.qagmediarating>0) {
        writer.String("qagmediarating");
        serializer<decltype(bid.qagmediarating)>::serialize(bid.qagmediarating, writer);
    }
    if(bid.language.size()>0) {
        writer.String("language");
        serializer<decltype(bid.language)>::serialize(bid.language.data(), bid.language.size(), writer);
    }
    if(bid.dealid.size()>0) {
        writer.String("dealid");
        serializer<decltype(bid.dealid)>::serialize(bid.dealid.data(), bid.dealid.size(), writer);
    }
    if(bid.w>0) {
        writer.String("w");
        serializer<decltype(bid.w)>::serialize(bid.w, writer);
    }
    if(bid.h>0) {
        writer.String("h");
        serializer<decltype(bid.h)>::serialize(bid.h, writer);
    }
    if(bid.wratio>0) {
        writer.String("wratio");
        serializer<decltype(bid.wratio)>::serialize(bid.wratio, writer);
    }
    if(bid.hratio>0) {
        writer.String("hratio");
        serializer<decltype(bid.hratio)>::serialize(bid.hratio, writer);
    }

    writer.EndObject();
}
};

} //namespace DSL

#endif /* RTB_DSL_RAPID_SERIALIZER_HPP */

