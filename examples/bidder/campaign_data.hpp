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
#include <iterator>


//This struct gets stored in the cache
struct CampaignData {
    uint32_t campaign_id;
    uint32_t ad_id;
   
    struct campaign_id_tag{};

    CampaignData(const CampaignData &data) :
        campaign_id{data.campaign_id} , ad_id{data.ad_id}
    {}
     
    CampaignData() :
        campaign_id{} , ad_id{}
    {}

    template<typename Alloc>
    CampaignData( const Alloc &) :
        campaign_id{} , ad_id{}
    {}

    CampaignData(uint32_t campaign_id, uint32_t ad_id) :
        campaign_id{campaign_id,} , ad_id{ad_id}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const  CampaignData & value)  {
        os << "{" << value.campaign_id << "|"
           << value.ad_id << "}";
        return os;
    } 
    friend std::ostream &operator<<(std::ostream & os, const  std::vector<CampaignData> & value)  {
        std::copy(std::begin(value), std::end(value), std::ostream_iterator<CampaignData>(os, " "));
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
        data.ad_id = boost::lexical_cast<uint32_t>(fields.at(1).begin(), fields.at(1).size());
        return is;
    }

    template<typename Key>
    void store(Key && key, const CampaignData  & data)  {
        campaign_id = key.template get<campaign_id_tag>();
        ad_id =  data.ad_id;
    }

    void retrieve(CampaignData  &data) const {
        data.campaign_id=campaign_id;
        data.ad_id=ad_id;
    }

    void operator()(CampaignData &entry) const {
        entry.campaign_id=campaign_id;
        entry.ad_id=ad_id;
    }

};

namespace ipc { namespace data {

template<typename Alloc>
using campaign_data_container =
boost::multi_index_container<
    CampaignData,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename CampaignData::campaign_id_tag>,
            boost::multi_index::composite_key<
              CampaignData,
              BOOST_MULTI_INDEX_MEMBER(CampaignData,uint32_t,campaign_id),
              BOOST_MULTI_INDEX_MEMBER(CampaignData,uint32_t,ad_id)
            >
        >
    >,
    boost::interprocess::allocator<CampaignData,typename Alloc::segment_manager>
> ;

}}

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::campaign_data_container>::char_allocator >
class CampaignDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::campaign_data_container> ;
        using CampaignTag = typename CampaignData::campaign_id_tag;
        using Keys = vanilla::tagged_tuple<CampaignTag, uint32_t>;
    public:    
        using CampaignDataCollection = std::vector<CampaignData>;
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
        
        bool retrieve(CampaignDataCollection &campaigns, uint32_t campaign_id) {
            auto p = cache.template retrieve_raw<CampaignTag>(campaign_id);
            auto is_found = p.first != p.second;
            campaigns.reserve(500);
            while ( p.first != p.second ) {
                CampaignData data;
                p.first->retrieve(data);
                campaigns.emplace_back(std::move(data));
                ++p.first;
            }
            return is_found;
        }

    private:
        const Config &config;
        Cache cache;
};

#endif /* CAMPAIGN_DATA_HPP */

