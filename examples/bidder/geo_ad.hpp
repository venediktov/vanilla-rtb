/* 
 * File:   geo_ad.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:19
 */


#ifndef GEO_AD_HPP
#define GEO_AD_HPP

#include "config.hpp"
#include "rtb/common/split_string.hpp"
#include "rtb/core/tagged_tuple.hpp"
#include "rtb/datacache/entity_cache.hpp"
#include "rtb/datacache/memory_types.hpp"
#include "examples/datacache/geo_entity.hpp"

#if BOOST_VERSION <= 106000
#include <boost/utility/string_ref.hpp>
namespace boost {
    using string_view = string_ref;
}
#else
#include <boost/utility/string_view.hpp>
#endif
#include <boost/lexical_cast.hpp>

/* 
 * Geo Targeting is implemented this way to test selection and reload on big amount of records
 */
struct GeoAd {
    std::string ad_id;
    uint32_t geo_id;
    std::string record;
   
    GeoAd(std::string ad_id, uint32_t geo_id) : 
        ad_id{std::move(ad_id)}, geo_id{geo_id}, record{}
    {}
    GeoAd():
        ad_id{}, geo_id{}, record{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<GeoAd> &geo_ad_ptr)  {
        os <<  *geo_ad_ptr;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  GeoAd & value)  {
        os << value.ad_id << "|" 
           << value.geo_id
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, GeoAd &l) {
        if ( !std::getline(is, l.record) ){
            return is;
        }
        std::vector<boost::string_view> fields;
        vanilla::common::split_string(fields, l.record, "\t");
        if(fields.size() < 2) {
            return is;
        }
        l.ad_id.assign(fields.at(0).begin(), fields.at(0).size()); 
        l.geo_id = boost::lexical_cast<int>(fields.at(1).begin(), fields.at(1).size());
        return is;
    }
};


template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::geo_container>::char_allocator >
class GeoAdDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::geo_container> ; 
        using Keys = vanilla::tagged_tuple< 
            typename ipc::data::geo_entity<Alloc>::geo_id_tag,   uint32_t
        >;
        using GeoTag = typename ipc::data::geo_entity<Alloc>::geo_id_tag;
    public:    
        using DataVect = std::vector<GeoAd>;

        GeoAdDataEntity(const Config &config):
            config{config}, cache(config.data().geo_ad_ipc_name)
        {}

    void load() noexcept(false) {
        std::ifstream in{config.data().geo_ad_source};
        if (!in) {
          throw std::runtime_error(std::string("could not open file ") + config.data().geo_ad_source + " exiting...");
        }
        LOG(debug) << "File opened " << config.data().geo_ad_source;
        cache.clear();
 
        std::for_each(std::istream_iterator<GeoAd>(in), std::istream_iterator<GeoAd>(), [&](const GeoAd &geo_ad) {
           if (!cache.insert(Keys{geo_ad.geo_id}, geo_ad) ) {
              LOG(debug) << "Failed to insert geo_ad=" << geo_ad;
           }
        });
    }

    bool retrieve(DataVect &geo_ads, uint32_t geo_id) {
        auto p = cache.template retrieve_raw<GeoTag>(geo_id);
        auto is_found = p.first != p.second;
        //TODO: random_access<> #include <boost/multi_index/random_access_index.hpp
        //this should give us ability to do geo_ads.reserve(std::distance(p.first,p.second))
        geo_ads.reserve(500);
        while ( p.first != p.second ) {
            GeoAd data;
            p.first->retrieve(data);
            geo_ads.emplace_back(std::move(data));
            ++p.first;
        }

        return is_found;
    }

    private:
        const Config &config;
        Cache cache;
};


#endif /* GEO_AD_HPP */

