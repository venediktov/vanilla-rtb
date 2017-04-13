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
#include "rtb/datacache/geo_entity.hpp"

struct GeoCampaign {
    uint32_t geo_id;
    uint32_t campaign_id;
    std::string record;
   
    GeoCampaign(uint32_t geo_id, uint32_t campaign_id) : 
        geo_id{geo_id}, campaign_id{campaign_id}, record{}
    {}
    GeoCampaign():
        geo_id{}, campaign_id{}, record{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<GeoCampaign> &geo_campaign_ptr)  {
        os <<  *geo_campaign_ptr;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  GeoCampaign & value)  {
        os << value.geo_id << "|" 
           << value.campaign_id
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, GeoCampaign &l) {
        if ( !std::getline(is, l.record) ){
            return is;
        }
        std::vector<boost::string_view> fields;
        vanilla::common::split_string(fields, l.record, "\t");
        if(fields.size() < 2) {
            return is;
        }
        l.geo_id = boost::lexical_cast<uint32_t>(fields.at(0).begin(), fields.at(0).size());
        l.campaign_id = boost::lexical_cast<uint32_t>(fields.at(1).begin(), fields.at(1).size());
        
        return is;
    }
};

//This struct gets stored in the cache
struct GeoCampaigns {
    using collection_type = std::set<uint32_t>;
    using iterator = collection_type::iterator;
    uint32_t geo_id;
    mutable collection_type campaign_ids;

    GeoCampaigns(const GeoCampaign &gc): 
        geo_id{gc.geo_id} , campaign_ids{gc.campaign_id}
    {}
     
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
    bool operator< (const GeoCampaigns &gcs) const {
        return geo_id < gcs.geo_id;
    }
    void operator+=(const GeoCampaign & gc) const {
        if ( geo_id == gc.geo_id) {
           campaign_ids.insert(gc.campaign_id);
        }
    }
};

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::geo_container>::char_allocator >
class GeoCampaignEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::geo_container> ; 
        using Keys = vanilla::tagged_tuple< 
            typename ipc::data::geo_entity<Alloc>::geo_id_tag,   uint32_t
        >;
        using GeoTag = typename ipc::data::geo_entity<Alloc>::geo_id_tag;
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
            
            std::set<GeoCampaigns> unique_geos;
            std::for_each(std::istream_iterator<GeoCampaign>(in), std::istream_iterator<GeoCampaign>(), [&](const GeoCampaign &geo_campaign) {
                GeoCampaigns geo_campaigns{geo_campaign};
                auto p = unique_geos.insert(geo_campaigns);
                if ( !p.second ) {
                    (*p.first) += geo_campaign;
                }
            });
            
            std::for_each(std::begin(unique_geos), std::end(unique_geos), [this](const GeoCampaigns &geo_campaigns){
                if (!cache.insert(Keys{geo_campaigns.geo_id}, geo_campaigns) ) {
                    LOG(debug) << "Failed to insert geo_campaign=" << geo_campaigns;    
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

