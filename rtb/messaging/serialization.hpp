#pragma once

#include <boost/serialization/vector.hpp>
#include <boost/serialization/optional.hpp>

//Non-Intrusive boost serialization implementation
namespace boost {
namespace serialization {


/******* BidRequest *************************************************************/
template<class Archive>
void serialize(Archive & ar, openrtb::BidRequest & value, const unsigned int version)
{

ar & value.id;
ar & value.imp;
ar & value.site;
ar & value.app;
ar & value.device;
ar & value.user;
ar & value.at;
ar & value.tmax;
ar & value.wseat;
ar & value.allimps;
ar & value.cur;
ar & value.bcat;
ar & value.badv;
ar & value.regs;
//ar & value.ext;
//ar & value.unparseable;

}


template<class Archive>
void serialize(Archive & ar, openrtb::Impression & value, const unsigned int version)
{

}


template<class Archive>
void serialize(Archive & ar, openrtb::Site & value, const unsigned int version)
{
}


template<class Archive>
void serialize(Archive & ar, openrtb::App & value, const unsigned int version)
{
}



template<class Archive>
void serialize(Archive & ar, openrtb::Device & value, const unsigned int version)
{
}


template<class Archive>
void serialize(Archive & ar, openrtb::User & value, const unsigned int version)
{
}


template<class Archive>
void serialize(Archive & ar, vanilla::unicode_string & value, const unsigned int version)
{    //ar & value;
}

template<class Archive>
void serialize(Archive & ar, openrtb::Regulations & value, const unsigned int version)
{
}



/******* BidResponse *************************************************************/

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
