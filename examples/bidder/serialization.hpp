/* 
 * File:   serialization.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:24
 */

#ifndef BIDDER_SERIALIZATION_HPP
#define BIDDER_SERIALIZATION_HPP

#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>

#include "examples/matchers/geo_campaign.hpp"
#include "campaign_data.hpp"
#include "examples/matchers/ad.hpp"
#include "examples/matchers/geo.hpp"
#include "examples/campaign/serialization.hpp"

//Non-Intrusive boost serialization implementation
namespace boost { namespace serialization {
    template<class Archive>
    void serialize(Archive & ar, Ad & value, [[maybe_unused]] const unsigned int version) {
        ar & value.ad_id;
        ar & value.campaign_id;
        ar & value.width;
        ar & value.height;
        ar & value.position;
        ar & value.max_bid_micros;
        ar & value.code;
    }

    template<class Archive>
    void serialize(Archive & ar, Geo & value, [[maybe_unused]] const unsigned int version) {
        ar & value.geo_id;
        ar & value.city;
        ar & value.country;
        ar & value.record;
    }
    template<class Archive>
    void serialize(Archive & ar, GeoCampaign & value, [[maybe_unused]] const unsigned int version) {
        ar & value.geo_id;
        ar & value.campaign_id;
    }
}} 

#endif /* BIDDER_SERIALIZATION_HPP */

