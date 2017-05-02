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
#include <iterator>

//This struct gets stored in the cache
struct GeoCampaign {
    uint32_t geo_id;
    uint32_t campaign_id;
     
    struct geo_id_tag{};

    GeoCampaign() :
        geo_id{} , campaign_id{}
    {}
     
    template<typename Alloc> 
    GeoCampaign(const Alloc &) :
        geo_id{} , campaign_id{}
    {}

    GeoCampaign(uint32_t geo_id, uint32_t campaign_id) :
        geo_id{geo_id} , campaign_id{campaign_id}
    {}

    friend std::ostream &operator<<(std::ostream & os, const  GeoCampaign & value)  {
        os << "{" << value.geo_id << "|" << value.campaign_id << "}" ;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  std::vector<GeoCampaign> & value)  {
        std::copy(std::begin(value), std::end(value), std::ostream_iterator<GeoCampaign>(os, " "));
    }
    friend std::istream &operator>>(std::istream &is, GeoCampaign &data) {
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
        data.campaign_id = boost::lexical_cast<uint32_t>(fields.at(1).begin(), fields.at(1).size());
        return is;
    }


    template<typename Key>
    void store(Key && key, const GeoCampaign  & data)  {
        geo_id = key.template get<geo_id_tag>();
        campaign_id =  data.campaign_id;
    }

    void retrieve(GeoCampaign  & data) const {
        data.geo_id=geo_id;
        data.campaign_id=campaign_id;
    }

    void operator()(GeoCampaign &entry) const {
        entry.geo_id=geo_id;
        entry.campaign_id=campaign_id;
    }

};


namespace ipc { namespace data {

template<typename Alloc>
using geo_campaign_container =
boost::multi_index_container<
    GeoCampaign,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename GeoCampaign::geo_id_tag>,
            boost::multi_index::composite_key<
              GeoCampaign,
              BOOST_MULTI_INDEX_MEMBER(GeoCampaign,uint32_t,geo_id),
              BOOST_MULTI_INDEX_MEMBER(GeoCampaign,uint32_t,campaign_id)
            >
        >
    >,
    boost::interprocess::allocator<GeoCampaign,typename Alloc::segment_manager>
> ;

}}

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::geo_campaign_container>::char_allocator >
class GeoCampaignEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::geo_campaign_container> ;
        using GeoTag = typename GeoCampaign::geo_id_tag;
        using Keys = vanilla::tagged_tuple<GeoTag, uint32_t>;
    public:
        using GeoCampaignCollection = std::vector<GeoCampaign>;
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
            
            std::for_each(std::istream_iterator<GeoCampaign>(in), std::istream_iterator<GeoCampaign>(), [this](const GeoCampaign &data) {
                if (!this->cache.insert(Keys{data.geo_id}, data)) {
                    LOG(debug) << "Failed to insert geo_campaign=" << data;
                }
            });
        }
        
        bool retrieve(GeoCampaignCollection &geo_campaigns, uint32_t geo_id) {
            auto p = cache.template retrieve_raw<GeoTag>(geo_id);
            auto is_found = p.first != p.second;
            geo_campaigns.reserve(500);
            while ( p.first != p.second ) {
                GeoCampaign data;
                p.first->retrieve(data);
                geo_campaigns.emplace_back(std::move(data));
                ++p.first;
            }
            return is_found;
        }

    private:
        const Config &config;
        Cache cache;
};


#endif /* GEO_CAMPAIGN_HPP */

