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
    void serialize(Archive & ar, CampaignBudget & value, const unsigned int) {
        ar & value.campain_id;
        ar & value.day_budget_limit;
        ar & value.day_budget_spent;
        ar & value.day_show_limit;
        ar & value.day_click_limit;
    }
}} 

#endif /* CAMPAIGN_SERIALIZATION_HPP */

