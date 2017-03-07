#pragma once

#include <boost/serialization/vector.hpp>
#include <boost/serialization/optional.hpp>

//Non-Intrusive boost serialization implementation
namespace boost {
    namespace serialization {

        /******* BidRequest *************************************************************/
        template<class Archive>
        void serialize(Archive & ar, openrtb::BidRequest & value, const unsigned int version) {
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
        void serialize(Archive & ar, openrtb::Impression & value, const unsigned int version) {
            ar & value.id;
            ar & value.banner;
            ar & value.video;
            ar & value.native;
            ar & value.displaymanager;
            ar & value.displaymanagerver;
            ar & value.instl;
            ar & value.tagid;
            ar & value.bidfloor;
            ar & value.bidfloorcur;
            ar & value.secure;
            ar & value.iframebuster;
            ar & value.pmp;
            //ar & value.ext;
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::MimeType & value, const unsigned int version) {
            ar & value.type;
        }
        
        template<class Archive>
        void serialize(Archive & ar, openrtb::Banner & value, const unsigned int version) {
            ar & value.w;
            ar & value.h;
            ar & value.wmax;
            ar & value.hmax;
            ar & value.wmin;
            ar & value.hmin;
            ar & value.id;
            ar & value.pos;
            ar & value.btype;
            ar & value.battr;
            ar & value.mimes;
            ar & value.topframe;
            ar & value.expdir;
            ar & value.api;
            //ar & value.ext;           
        }
        
        template<class Archive>
        void serialize(Archive & ar, openrtb::Video & value, const unsigned int version) {
            ar & value.mimes;
            ar & value.minduration;
            ar & value.maxduration;
            ar & value.protocols;
            ar & value.protocol;
            ar & value.w;
            ar & value.h;
            ar & value.startdelay;
            ar & value.placement;
            ar & value.linearity;
            ar & value.skip;
            ar & value.skipmin;
            ar & value.skipafter;
            ar & value.sequence;
            ar & value.battr;

            ar & value.maxextended;
            ar & value.minbitrate;
            ar & value.maxbitrate;
            ar & value.boxingallowed;

            ar & value.playbackmethod;
            ar & value.playbackend;
            ar & value.delivery;
            ar & value.pos;
            ar & value.companiontype;
            //jsonv::value ext
        }
        
        template<class Archive>
        void serialize(Archive & ar, openrtb::Native & value, const unsigned int version) {
        }
        
        template<class Archive>
        void serialize(Archive & ar, openrtb::PMP & value, const unsigned int version) {
        }
        
        template<class Archive>
        void serialize(Archive & ar, openrtb::Site & value, const unsigned int version) {
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::App & value, const unsigned int version) {
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::Device & value, const unsigned int version) {
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::User & value, const unsigned int version) {
            ar & value.yob;
            ar & value.id;
            ar & value.buyeruid;
            ar & value.gender;
            ar & value.keywords;
            ar & value.customdata;
            ar & value.geo;
            ar & value.data;
            //ar & value.ext;
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::Geo & value, const unsigned int version) { //ar & value;
            ar & value.lat;
            ar & value.lon;
            ar & value.type;
            ar & value.utcoffset;
            ar & value.city;
            ar & value.country;
            ar & value.region;
            ar & value.regionfips104;                                     
            ar & value.metro;                             
            ar & value.zip;
            //ar & value.ext;
        }
        
        template<class Archive>
        void serialize(Archive & ar, openrtb::UserData & value, const unsigned int version) { //ar & value;
            ar & value.id;
            ar & value.name;
            //ar & value.segment;
            //ar & value.ext;
        }
        template<class Archive>
        void serialize(Archive & ar, vanilla::unicode_string & value, const unsigned int version) { //ar & value;
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::Regulations & value, const unsigned int version) {
        }

        /******* BidResponse *************************************************************/

        template<class Archive>
        void serialize(Archive & ar, openrtb::BidResponse & value, const unsigned int version) {
            ar & value.id;
            ar & value.seatbid;
            ar & value.bidid;
            ar & value.cur;
            ar & value.customdata;
            ar & value.nbr;
            //ar & value.ext //TODO: for this we need template <class Archive> load() and save() 
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::SeatBid & value, const unsigned int version) {
            ar & value.bid;
            ar & value.seat;
            ar & value.group;
            //ar & value.ext //TODO: for this we need template <class Archive> load() and save() 
        }

        template<class Archive>
        void serialize(Archive & ar, openrtb::Bid & value, const unsigned int version) {
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
