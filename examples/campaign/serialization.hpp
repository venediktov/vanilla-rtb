/* 
 * File:   serialization.hpp
 * Author: Vladimir Venediktov
 *
 * Created on March 12, 2017, 10:20 PM
 */

#ifndef CAMPAIGN_SERIALIZATION_HPP
#define CAMPAIGN_SERIALIZATION_HPP


//Non-Intrusive boost serialization implementation
namespace boost { namespace serialization {
    template<class Archive>
    void serialize(Archive & ar, vanilla::CampaignBudget & value, const unsigned int) {
        ar & value.campaign_id;
        ar & value.day_budget_limit;
        ar & value.day_budget_spent;
        ar & value.metric;
    }
    template<class Archive>
    void serialize(Archive & ar, vanilla::CampaignBudget::Metric & metric, const unsigned int) {
        ar & metric.type;
        ar & metric.value;
    }
}} 

#endif /* CAMPAIGN_SERIALIZATION_HPP */

