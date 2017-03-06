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

//This struct gets stored in the cache
struct GeoAds {
    using collection_type = std::set<std::string> ;
    using iterator = collection_type::iterator;
    uint32_t geo_id;
    mutable collection_type ad_ids;

    GeoAds(const GeoAd &ad): 
        geo_id{ad.geo_id} , ad_ids{ad.ad_id}
    {}
     
    GeoAds():
        geo_id{} , ad_ids{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<GeoAds> &geo_ad_ptr)  {
        os <<  *geo_ad_ptr;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  GeoAds & value)  {
        os << value.geo_id
        ;
        return os;
    } 
    bool operator< (const GeoAds &gas) const {
        return geo_id < gas.geo_id;
    }
    void operator+=(const GeoAd & geo_ad) const {
        if ( geo_id == geo_ad.geo_id) {
           ad_ids.insert(geo_ad.ad_id);
        }
    }
};

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared,
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::geo_ad_container>::char_allocator >
class GeoAdDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::geo_ad_container> ; 
        using Keys = vanilla::tagged_tuple< 
            typename ipc::data::geo_ad_entity<Alloc>::geo_id_tag,   uint32_t
        >;
        using DataVect = std::vector<std::shared_ptr <GeoAd> >;
        using GeoTag = typename ipc::data::geo_ad_entity<Alloc>::geo_id_tag;
    public:    
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
            
            std::set<GeoAds> unique_geos;
            std::for_each(std::istream_iterator<GeoAd>(in), std::istream_iterator<GeoAd>(), [&](const GeoAd &geo_ad) {
                GeoAds geo_ads{geo_ad};
                auto p = unique_geos.insert(geo_ads);
                if ( !p.second ) {
                    (*p.first) += geo_ad;
                }
            });
            
            std::for_each(std::begin(unique_geos), std::end(unique_geos), [this](const GeoAds &geo_ads){
                if (!cache.insert(Keys{geo_ads.geo_id}, geo_ads) ) {
                    LOG(debug) << "Failed to insert geo_ad=" << geo_ads;    
                }
            });
        }
        
        bool retrieve(DataVect &vect, uint32_t geo_id) {
            bool result = false;
            std::vector<std::shared_ptr<GeoAds>> geo_ads;
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "geo_ads");
                result = cache.template retrieve<GeoTag>(geo_ads, geo_id);
                if(result) {
                    auto geo_id = geo_ads[0]->geo_id;
                    auto ads = geo_ads[0]->ad_ids;
                    std::transform(std::begin(ads), std::end(ads), std::back_inserter(vect), [geo_id](const std::string &ad_id) {
                        return std::make_shared<GeoAd>(std::move(ad_id),geo_id);
                    });
                }
            }
            LOG(debug) << sp->str();
            return result;
        }
    private:
        const Config &config;
        Cache cache;
};


#endif /* GEO_AD_HPP */

