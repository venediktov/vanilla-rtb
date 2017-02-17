/* 
 * File:   geo_ad.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:19
 */

#ifndef AD_GEO_HPP
#define AD_GEO_HPP

#include "config.hpp"
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
        std::vector<std::string> fields;
        boost::split(fields, l.record, boost::is_any_of("\t"), boost::token_compress_on);
        if(fields.size() < 2) {
            std::cout << "Not enought fields " << l.record << std::endl;
            return is;
        }
        l.ad_id = fields.at(0); 
        l.geo_id = atoi(fields.at(1).c_str());
        return is;
    }
};

template <typename Memory = typename mpclmi::ipc::Shared, 
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::geo_ad_container>::char_allocator >
class GeoAdDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::geo_ad_container> ; 
        using Keys = vanilla::tagged_tuple<
            typename ipc::data::geo_ad_entity<Alloc>::ad_id_tag,    std::string, 
            typename ipc::data::geo_ad_entity<Alloc>::geo_id_tag,   uint32_t
        >;
        using DataVect = std::vector<std::shared_ptr <GeoAd> >;
        using GeoAdTag = typename ipc::data::geo_ad_entity<Alloc>::geo_ad_tag;
        using GeoTag = typename ipc::data::geo_ad_entity<Alloc>::geo_id_tag;
    public:    
        GeoAdDataEntity(const BidderConfig &config):
            config{config}, cache(config.data().geo_ad_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().geo_ad_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().geo_ad_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().geo_ad_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<GeoAd>(in), std::istream_iterator<GeoAd>(), [&](const GeoAd &geo_ad){
                cache.insert(Keys{geo_ad.ad_id, geo_ad.geo_id}, geo_ad);
            });            
        }
        
        bool retrieve(DataVect &vect, uint32_t geo_id) {
            return cache.template retrieve<GeoAdTag>(vect, geo_id);
        }
    private:
        const BidderConfig &config;
        Cache cache;
};


#endif /* AD_GEO_HPP */

