/* 
 * File:   geo_campaign.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:19
 */

#ifndef GEO_CAMPAIGN_HPP
#define GEO_CAMPAIGN_HPP

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
struct GeoCampaigns {
    using collection_type = std::vector<uint32_t>;
    using iterator = collection_type::iterator;
    uint32_t geo_id;
    mutable collection_type campaign_ids;
     
    GeoCampaigns():
        geo_id{} , campaign_ids{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<GeoCampaigns> &geo_campaign_ptr)  {
        os <<  *geo_campaign_ptr;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  GeoCampaigns & value)  {
        os << value.geo_id
        ;
        return os;
    } 
    friend std::istream &operator>>(std::istream &is, GeoCampaigns &data) {
        std::string record;
        if (!std::getline(is, record) ){
            return is;
        }
        std::vector<boost::string_view> fields;
        vanilla::common::split_string(fields, record, "\t");
        if(fields.size() < 2) {
            return is;
        }
        data.geo_id = boost::lexical_cast<uint32_t>(fields.at(0).begin(), fields.at(0).size());
        uint32_t count = boost::lexical_cast<uint32_t>(fields.at(1).begin(), fields.at(1).size());
        data.campaign_ids.clear();
        data.campaign_ids.reserve(count);
        
        uint32_t end_idx = count + 2;
        for(uint32_t fields_idx = 2; fields_idx < end_idx; fields_idx++) {
            data.campaign_ids.push_back(boost::lexical_cast<uint32_t>(fields.at(fields_idx).begin(), fields.at(fields_idx).size()));
        }        
        return is;
    }
    bool operator< (const GeoCampaigns &gcs) const {
        return geo_id < gcs.geo_id;
    }
    void clear() {
        geo_id = 0;
        campaign_ids.clear();
    }
};

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::campaign_container>::char_allocator >
class GeoCampaignEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::campaign_container> ; 
        using Keys = vanilla::tagged_tuple< 
            typename ipc::data::campaign_entity<Alloc>::campaign_id_tag,   uint32_t
        >;
        using GeoTag = typename ipc::data::campaign_entity<Alloc>::campaign_id_tag;
    public:    
        GeoCampaignEntity(const Config &config):
            config{config}, cache(config.data().geo_campaign_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().geo_campaign_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().geo_campaign_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().geo_campaign_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<GeoCampaigns>(in), std::istream_iterator<GeoCampaigns>(), [this](const GeoCampaigns &data) {
                if (!this->cache.insert(Keys{data.geo_id}, data) ) {
                    LOG(debug) << "Failed to insert geo_campaign=" << data.geo_id;    
                }
            });
        }
        
        bool retrieve(GeoCampaigns &geo_campaigns, uint32_t geo_id) {
            bool result = false;
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "geo_campaigns");
                result = cache.template retrieve<GeoTag>(geo_campaigns, geo_id);
            }
            LOG(debug) << sp->str();
            return result;
        }
    private:
        const Config &config;
        Cache cache;
};


#endif /* GEO_CAMPAIGN_HPP */

