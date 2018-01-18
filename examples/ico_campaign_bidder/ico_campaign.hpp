/* 
 * File:   ico_campaign.hpp
 *
 */

#ifndef ICO_CAMPAIGN_HPP
#define ICO_CAMPAIGN_HPP

#include "config.hpp"
#include "rtb/common/split_string.hpp"
#include "core/tagged_tuple.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/lexical_cast.hpp>
#include <iterator>

//This struct gets stored in the cache
struct ICOCampaign {
    uint32_t ref_id;
    uint32_t campaign_id;
     
    struct ref_id_tag{};

    ICOCampaign() :
        ref_id{} , campaign_id{}
    {}
     
    template<typename Alloc> 
    ICOCampaign(const Alloc &) :
        ref_id{} , campaign_id{}
    {}

    ICOCampaign(uint32_t ref_id, uint32_t campaign_id) :
        ref_id{ref_id} , campaign_id{campaign_id}
    {}

    friend std::ostream &operator<<(std::ostream & os, const  ICOCampaign & value)  {
        os << "{" << value.ref_id << "|" << value.campaign_id << "}" ;
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
        std::vector<boost::string_view> fields;
        vanilla::common::split_string(fields, record, "\t");
        if(fields.size() < 2) {
            return is;
        }
        data.ref_id = boost::lexical_cast<uint32_t>(fields.at(0).begin(), fields.at(0).size());
        data.campaign_id = boost::lexical_cast<uint32_t>(fields.at(1).begin(), fields.at(1).size());
        return is;
    }


    template<typename Key>
    void store(Key && key, const ICOCampaign  & data)  {
        ref_id = key.template get<ref_id_tag>();
        campaign_id =  data.campaign_id;
    }

    void retrieve(ICOCampaign  & data) const {
        data.ref_id=ref_id;
        data.campaign_id=campaign_id;
    }

    void operator()(ICOCampaign &entry) const {
        entry.ref_id=ref_id;
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
            boost::multi_index::tag<typename ICOCampaign::ref_id_tag>,
            boost::multi_index::composite_key<
              ICOCampaign,
              BOOST_MULTI_INDEX_MEMBER(ICOCampaign,uint32_t,ref_id),
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
        using RefererTag = typename ICOCampaign::ref_id_tag;
        using Keys = vanilla::tagged_tuple<RefererTag, uint32_t>;
    public:
        using ICOCampaignCollection = std::vector<ICOCampaign>;
        using type = ICOCampaignCollection;
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
                if (!this->cache.insert(Keys{data.ref_id}, data).second) {
                    LOG(debug) << "Failed to insert ico_campaign=" << data;
                }
            });
        }
        
        bool retrieve(ICOCampaignCollection &ico_campaigns, uint32_t ref_id) {
            auto p = cache.template retrieve_raw<RefererTag>(ref_id);
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

