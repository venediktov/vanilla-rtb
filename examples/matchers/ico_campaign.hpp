/* 
 * File:   ico_campaign.hpp
 *
 */

#pragma once
#ifndef ICO_CAMPAIGN_HPP
#define ICO_CAMPAIGN_HPP

#include <string>
#include <cstdint>
#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include "rtb/datacache/any_str_ops.hpp"
#include "rtb/common/split_string.hpp"
#include "core/tagged_tuple.hpp"
#include "../bidder_experimental/config.hpp"

#if __APPLE_CC__
#if __cplusplus >= 201402L
#define STRING_VIEW_DEFINED 1
#endif
#elif __GNUC__
#if __cplusplus >= 201703L
#define STRING_VIEW_DEFINED 1
#endif
#endif

//This struct gets stored in the cache
struct ICOCampaign {
    uint32_t domain_id;
    uint32_t campaign_id;
     
    struct domain_id_tag{};

    ICOCampaign() :
        domain_id{} , campaign_id{}
    {}
     
    template<typename Alloc> 
    ICOCampaign(const Alloc &) :
        domain_id{} , campaign_id{}
    {}

    ICOCampaign(uint32_t ref_id, uint32_t campaign_id) :
        domain_id{ref_id} , campaign_id{campaign_id}
    {}

    friend std::ostream &operator<<(std::ostream & os, const  ICOCampaign & value)  {
        os << "{" << value.domain_id << "|" << value.campaign_id << "}" ;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  std::vector<ICOCampaign> & value)  {
        std::copy(std::begin(value), std::end(value), std::ostream_iterator<ICOCampaign>(os, " "));
        return os;
    }
    friend std::istream &operator>>(std::istream &is, ICOCampaign &data) {
        std::string record;
        if (!std::getline(is, record) ){
            return is;
        }
#if defined(STRING_VIEW_DEFINED)
        std::vector<std::string_view> fields ;
        vanilla::common::split_string(fields, record, "\t");
#else
        std::vector<std::string> fields ;
        boost::split(fields, record, boost::is_any_of("\t"), boost::token_compress_on);
#endif
        if(fields.size() < 2) {
            return is;
        }
#if defined(STRING_VIEW_DEFINED)
        data.domain_id = boost::lexical_cast<uint32_t>(fields.at(0));
        data.campaign_id = boost::lexical_cast<uint32_t>(fields.at(1));
#else
        data.domain_id = boost::lexical_cast<uint32_t>(fields.at(0).begin(), fields.at(0).size());
        data.campaign_id = boost::lexical_cast<uint32_t>(fields.at(1).begin(), fields.at(1).size());
#endif
        return is;
    }


    template<typename Key>
    void store(Key && key, const ICOCampaign  & data)  {
        domain_id = key.template get<domain_id_tag>();
        campaign_id =  data.campaign_id;
    }

    void retrieve(ICOCampaign  & data) const {
        data.domain_id=domain_id;
        data.campaign_id=campaign_id;
    }

    void operator()(ICOCampaign &entry) const {
        entry.domain_id=domain_id;
        entry.campaign_id=campaign_id;
    }

};


namespace ipc { namespace data {

template<typename Alloc>
using ico_campaign_container =
boost::multi_index_container<
    ICOCampaign,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename ICOCampaign::domain_id_tag>,
            boost::multi_index::composite_key<
              ICOCampaign,
              BOOST_MULTI_INDEX_MEMBER(ICOCampaign,uint32_t,domain_id),
              BOOST_MULTI_INDEX_MEMBER(ICOCampaign,uint32_t,campaign_id)
            >
        >
    >,
    boost::interprocess::allocator<ICOCampaign,typename Alloc::segment_manager>
> ;

}}

template <typename Config=vanilla::config::config<ico_bidder_config_data>,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::ico_campaign_container>::char_allocator >
class ICOCampaignEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::ico_campaign_container> ;
        using DomainTag = typename ICOCampaign::domain_id_tag;
        using Keys = vanilla::tagged_tuple<DomainTag, uint32_t>;
    public:
        using ICOCampaignCollection = std::vector<ICOCampaign>;
        ICOCampaignEntity(const Config &config):
            config{config}, cache(config.data().ico_campaign_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().ico_campaign_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().ico_campaign_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().ico_campaign_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<ICOCampaign>(in), std::istream_iterator<ICOCampaign>(), [this](const ICOCampaign &data) {
                if (!this->cache.insert(Keys{data.domain_id}, data).second) {
                    LOG(debug) << "Failed to insert ico_campaign=" << data;
                }
            });
        }
        
        bool retrieve(ICOCampaignCollection &ico_campaigns, uint32_t domain_id) {
            auto p = cache.template retrieve_raw<DomainTag>(domain_id);
            auto is_found = p.first != p.second;
            ico_campaigns.reserve(500);
            while ( p.first != p.second ) {
                ICOCampaign data;
                p.first->retrieve(data);
                ico_campaigns.emplace_back(std::move(data));
                ++p.first;
            }
            return is_found;
        }

    private:
        const Config &config;
        Cache cache;
};


#endif /* ICO_CAMPAIGN_HPP */

