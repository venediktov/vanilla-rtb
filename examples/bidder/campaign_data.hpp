/* 
 * File:   campaign_data.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 6 апреля 2017 г., 23:45
 */

#ifndef CAMPAIGN_DATA_HPP
#define CAMPAIGN_DATA_HPP

#include "config.hpp"
#include "rtb/common/split_string.hpp"
#include "core/tagged_tuple.hpp"
#if BOOST_VERSION <= 106000
#include <boost/utility/string_ref.hpp>
namespace boost {
    using string_view = string_ref;
}
#else
#include <boost/utility/string_view.hpp>
#endif
#include <boost/lexical_cast.hpp>
#include "rtb/datacache/campaign_entity.hpp"


//This struct gets stored in the cache
struct CampaignData {
    using collection_type = std::vector<uint64_t> ;
    using iterator = collection_type::iterator;
    uint32_t campaign_id;
    mutable collection_type ad_ids;

    CampaignData(const CampaignData &data): 
        campaign_id{data.campaign_id} , ad_ids{data.ad_ids}
    {}
     
    CampaignData():
        campaign_id{} , ad_ids{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<CampaignData> &campaign_data_ptr)  {
        os <<  *campaign_data_ptr;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  CampaignData & value)  {
        os << value.campaign_id;
        for(auto &id : value.ad_ids) {
            os << "|" << id;
        }
        return os;
    } 
    friend std::istream &operator>>(std::istream &is, CampaignData &data) {
        std::string record;
        if (!std::getline(is, record) ){
            return is;
        }
        std::vector<boost::string_view> fields;
        vanilla::common::split_string(fields, record, "\t");
        if(fields.size() < 2) {
            return is;
        }
        data.campaign_id = boost::lexical_cast<uint32_t>(fields.at(0).begin(), fields.at(0).size());
        uint32_t ads_count = boost::lexical_cast<uint32_t>(fields.at(1).begin(), fields.at(1).size());
        data.ad_ids.clear();
        data.ad_ids.reserve(ads_count);
        //data.ad_ids.reserve(ads_count);
        uint32_t ads_end_idx = ads_count + 2;
        for(uint32_t fields_idx = 2; fields_idx < ads_end_idx; fields_idx++) {
            //data.ad_ids.push_back(boost::lexical_cast<uint32_t>(fields.at(fields_idx).begin(), fields.at(fields_idx).size()));
            data.ad_ids.push_back(boost::lexical_cast<uint64_t>(fields.at(fields_idx).begin(), fields.at(fields_idx).size()));
        }        
        return is;
    }
    bool operator< (const CampaignData &data) const {
        return campaign_id < data.campaign_id;
    }
    void clear() {
        campaign_id = 0;
        ad_ids.clear();
    }
};

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::campaign_container>::char_allocator >
class CampaignDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::campaign_container> ; 
        using Keys = vanilla::tagged_tuple< 
            typename ipc::data::campaign_entity<Alloc>::campaign_id_tag,   uint32_t
        >;
        using CampaignTag = typename ipc::data::campaign_entity<Alloc>::campaign_id_tag;
    public:    
        CampaignDataEntity(const Config &config):
            config{config}, cache(config.data().campaign_data_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().campaign_data_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().campaign_data_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().campaign_data_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<CampaignData>(in), std::istream_iterator<CampaignData>(), [this](const CampaignData &data) {
                if (!this->cache.insert(Keys{data.campaign_id}, data) ) {
                    LOG(debug) << "Failed to insert campaign_data=" << data.campaign_id;    
                }
            });
        }
        
        bool retrieve(CampaignData &data, uint32_t campaign_id) {
            bool result = false;
            //auto sp = std::make_shared<std::stringstream>();
            //{
                //perf_timer<std::stringstream> timer(sp, "campaign_data");
                result = cache.template retrieve<CampaignTag>(data, campaign_id);
            //}
            //LOG(debug) << sp->str();
            return result;
        }
    private:
        const Config &config;
        Cache cache;
};

#endif /* CAMPAIGN_DATA_HPP */

