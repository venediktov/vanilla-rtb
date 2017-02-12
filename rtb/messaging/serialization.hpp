#pragma once

#include <boost/serialization/vector.hpp>

//Non-Intrusive boost serialization implementation
namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, openrtb::BidResponse & value, const unsigned int version)
{
    ar & value.id;
    ar & value.seatbid;
    ar & value.bidid;
    ar & value.cur;
    ar & value.customdata;
    ar & value.nbr;
    //ar & value.ext //TODO: for this we need template <class Archive> load() and save() 
}

template<class Archive>
void serialize(Archive & ar, openrtb::SeatBid & value, const unsigned int version)
{
    ar & value.bid;
    ar & value.seat;
    ar & value.group;
    //ar & value.ext //TODO: for this we need template <class Archive> load() and save() 
}

template<class Archive>
void serialize(Archive & ar, openrtb::Bid & value, const unsigned int version)
{
    ar & value.id;
    ar & value.impid;
    ar & value.adid;
    ar & value.nurl;
    ar & value.adm;
    ar & value.adomain;
    ar & value.iurl;
    ar & value.cid;
    ar & value.crid;
    ar & value.attr;
    ar & value.dealid;
    ar & value.w;
    ar & value.h;
    //ar & value.ext //TODO: for this we need template <class Archive> load() and save() 
}

} // namespace serialization
} // namespace boost
