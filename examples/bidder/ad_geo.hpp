/* 
 * File:   ad_geo.hpp
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
struct AdGeo {
    std::string ad_id;
    uint32_t geo_id;
    std::string record;
    
    AdGeo(std::string ad_id, uint32_t geo_id) : 
        ad_id{std::move(ad_id)}, geo_id{geo_id}, record{}
    {}
    AdGeo():
        ad_id{}, geo_id{}, record{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<AdGeo> &ad_geo_ptr)  {
        os <<  *ad_geo_ptr;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  AdGeo & value)  {
        os << value.ad_id << "|" 
           << value.geo_id
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, AdGeo &l) {
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
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::ad_container>::char_allocator >
class AdGeoDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::ad_geo_container> ; 
        using Keys = vanilla::tagged_tuple<
            typename ipc::data::ad_geo_entity<Alloc>::ad_id_tag,    std::string, 
            typename ipc::data::ad_geo_entity<Alloc>::geo_id_tag,   uint32_t
        >;
        using DataVect = std::vector<std::shared_ptr <AdGeo> >;
        using AdGeoTag = typename ipc::data::ad_geo_entity<Alloc>::ad_geo_tag;
        using GeoTag = typename ipc::data::ad_geo_entity<Alloc>::geo_id_tag;
    public:    
        AdGeoDataEntity(const BidderConfig &config):
            config{config}, cache(config.data().ad_geo_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().ad_geo_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().ad_geo_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().ad_geo_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<AdGeo>(in), std::istream_iterator<AdGeo>(), [&](const AdGeo &ad_geo){
                cache.insert(Keys{ad_geo.ad_id, ad_geo.geo_id}, ad_geo);
            });            
        }
        
        bool retrieve(DataVect &vect, uint32_t geo_id) {
            return cache.template retrieve<GeoTag>(vect, geo_id);
        }
    private:
        const BidderConfig &config;
        Cache cache;
};


#endif /* AD_GEO_HPP */

