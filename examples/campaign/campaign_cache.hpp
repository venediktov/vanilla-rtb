/* 
 * File:   campaign_cache.hpp
 * Author: vladimir venediktov
 *
 * Created on March 12, 2017, 9:41 PM
 */

#ifndef CAMPAIGN_CACHE_HPP
#define	CAMPAIGN_CACHE_HPP

#include "config.hpp"
#include "core/tagged_tuple.hpp"
#include "common/perf_timer.hpp"
#include "datacache/campaign_entity.hpp"
#include "datacache/entity_cache.hpp"
#include "datacache/memory_types.hpp"
#include <boost/serialization/strong_typedef.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <memory>
#include <cstdint>
#include <iostream>
#include "rtb/common/split_string.hpp"

namespace vanilla {
    
namespace types {    
    BOOST_STRONG_TYPEDEF(uint64_t, budget)
    BOOST_STRONG_TYPEDEF(uint64_t, price )
}

struct CampaignBudget {

    enum class MetricType : int8_t {
        UNDEFINED = 0,
        CPM = 1,
        CPC = 2,
        CPA = 3
    };
    struct Metric {
        MetricType type{};
        uint64_t value{};
    };

    uint32_t campaign_id{};
    uint64_t day_budget_limit{}; //micro dollars
    uint64_t day_budget_spent{}; //micro dollars
    uint64_t day_budget_overdraft{}; //micro dollars
    uint64_t day_show_limit{}; //TODO: remove
    uint64_t day_click_limit{}; //TODO: remove
    Metric metric{};
    std::string record{};
   
    void update(types::budget value) {
        day_budget_limit = value;
    }
    void update(types::price value) {
        auto min_value = std::min(day_budget_limit, (uint64_t)value);
        day_budget_limit -= min_value;
        day_budget_spent += min_value;
        if (min_value < value) {
            day_budget_overdraft += value - min_value; 
        }
    }
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<CampaignBudget> ptr)  {
        os <<  *ptr;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const CampaignBudget & value)  {
        os << value.campaign_id      << "|" 
           << value.day_budget_limit << "|"
           << value.day_budget_spent << "|"
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, CampaignBudget &l) {
        if ( !std::getline(is, l.record) ){
            return is;
        }
        std::vector<std::string> fields;
        vanilla::common::split_string(fields, l.record, "\t");
        if(fields.size() < 5) {
            return is;
        }
        l.campaign_id = atol(fields.at(0).c_str()); 
        l.day_budget_limit = atol(fields.at(1).c_str());
        l.day_budget_spent = atol(fields.at(2).c_str());
        l.day_budget_overdraft = 0;
        auto day_show_limit = atol(fields.at(3).c_str());
        if ( day_show_limit ) {
            l.metric.type  = MetricType::CPM;
            l.metric.value = day_show_limit;
            return is;
        }
        auto day_click_limit = atol(fields.at(4).c_str());
        if ( day_click_limit ) {
            l.metric.type  = MetricType::CPC;
            l.metric.value = day_click_limit;
            return is;
        }

        return is;
    }
   
};

template <typename Config = CampaignManagerConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::campaign_container>::char_allocator >
class CampaignCache {
        using Cache = datacache::entity_cache<Memory, ipc::data::campaign_container> ; 
        using Keys = vanilla::tagged_tuple< 
            typename ipc::data::campaign_entity<Alloc>::campaign_id_tag,   uint32_t
        >;
        using CampaignTag = typename ipc::data::campaign_entity<Alloc>::campaign_id_tag;
    public:
        using DataCollection = std::vector<std::shared_ptr <CampaignBudget> >;
        CampaignCache(const Config &config):
            config{config}, cache(config.data().ipc_name)
        {}
        
        bool retrieve(DataCollection &data, uint32_t campaign_id) {
            bool result = false;
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "campaign_id");
                result = cache.template retrieve<CampaignTag>(data, campaign_id);
            }
            LOG(debug) << sp->str();
            return result;
        }
        bool retrieve(DataCollection &data) {
            bool result = false;
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "all_compaign_ids");
                result = cache.template retrieve(data);
            }
            LOG(debug) << sp->str();
            return result;
        }
        bool insert(const CampaignBudget &budget, uint32_t campaign_id) {
            return cache.insert(Keys{ campaign_id }, budget);
        }
        bool update(const CampaignBudget &budget, uint32_t campaign_id) {
            return cache.template update<CampaignTag>(Keys{ campaign_id }, budget, campaign_id);
        }
        bool update(const CampaignBudget &budget, uint32_t old_campaign_id, uint32_t new_campaign_id) {
            return cache.template update<CampaignTag>(Keys{ new_campaign_id }, budget, old_campaign_id);
        }
        bool remove(uint32_t campaign_id) {
            cache.template remove<CampaignTag>(campaign_id);
            return true;
        }
        void load() noexcept(false) {
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "ad");
                std::ifstream in{config.data().campaign_budget_source};
                if (!in) {
                    throw std::runtime_error(std::string("could not open file ") + config.data().campaign_budget_source + " exiting...");
                }
                LOG(debug) << "File opened " << config.data().campaign_budget_source;
                cache.clear();

                std::for_each(std::istream_iterator<CampaignBudget>(in), std::istream_iterator<CampaignBudget>(), [&](const CampaignBudget & c) {
                    cache.insert(Keys{c.campaign_id}, c);
                });
            }
            LOG(debug) << sp->str();
        }
    private:
        const Config &config;
        Cache cache;
};


} //vanilla

#endif	/* CAMPAIGN_CACHE_HPP */

