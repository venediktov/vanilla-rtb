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
#if BOOST_VERSION <= 106000
#include <boost/utility/string_ref.hpp>
namespace boost {
    using string_view = string_ref;
}
#else
#include <boost/utility/string_view.hpp>
#endif
#include <boost/lexical_cast.hpp>



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
        std::vector<boost::string_view> fields;
        vanilla::common::split_string(fields, l.record, "\t");
        if(fields.size() < 5) {
            return is;
        }
        l.campaign_id = boost::lexical_cast<uint32_t>(fields.at(0).begin(), fields.at(0).size());
        l.day_budget_limit = boost::lexical_cast<uint64_t>(fields.at(1).begin(), fields.at(1).size());
        l.day_budget_spent = boost::lexical_cast<uint64_t>(fields.at(2).begin(), fields.at(2).size());
        l.day_budget_overdraft = 0;
        l.metric.type = static_cast<MetricType>(boost::lexical_cast<uint32_t>(fields.at(3).begin(), fields.at(3).size()));
        l.metric.value = boost::lexical_cast<uint64_t>(fields.at(4).begin(), fields.at(4).size());
        return is;
    }
   
};

struct BudgetManager {
    uint64_t authorize (const CampaignBudget &budget) {
        if ( budget.day_budget_spent >= budget.day_budget_limit ) {
            return 0; //no bid HTTP/204 or empty seatbid
        }
        switch(budget.metric.type) {
            case CampaignBudget::MetricType::CPM :
                return std::min(budget.day_budget_limit - budget.day_budget_spent, budget.metric.value / 1000) ; //value is in micro $ 
            case CampaignBudget::MetricType::CPC :
                return std::min(budget.day_budget_limit - budget.day_budget_spent, budget.metric.value); // value is in micro $
            default :
                return 0; //TODO: when CPA implemeted review and maybe throw exception
        }
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
        //this interface used by vanilla::core::Banker    
        auto retrieve(uint32_t campaign_id) {
            CampaignBudget budget;
            cache.template retrieve<CampaignTag>(budget, campaign_id);
            return budget;
        }
        //this interface used by campaign_manager_test.cpp
        bool retrieve(DataCollection &data, uint32_t campaign_id) {
            return cache.template retrieve<CampaignTag>(data, campaign_id);
        }
        bool retrieve(DataCollection &data) {
            data.reserve(500);
            return cache.template retrieve(data);
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

