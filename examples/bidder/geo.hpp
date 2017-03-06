/* 
 * File:   geo.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:21
 */

#ifndef BIDDER_GEO_HPP
#define BIDDER_GEO_HPP

#include <string>
#include <cstdint>
#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "core/tagged_tuple.hpp"
#include "config.hpp"

struct Geo {
    uint32_t geo_id;
    std::string city;
    std::string country;
    std::string record;
    
    Geo(uint32_t geo_id, std::string city, std::string country) : 
        geo_id{geo_id}, city{std::move(city)}, country{std::move(country)}, record{}
    {}
        
    Geo():
        geo_id{}, city{}, country{}, record{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<Geo> &geo) {
        os << *geo;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const Geo &value)  {
        os << value.geo_id << "|" 
           << value.city << "|" 
           << value.country
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, Geo &l) {
        if ( !std::getline(is, l.record) ){
            return is;
        }
        std::vector<std::string> fields;
        boost::split(fields, l.record, boost::is_any_of("\t"), boost::token_compress_on);
        if(fields.size() < 3) {
            return is;
        }
        l.geo_id = atol(fields.at(0).c_str()); 
        l.city = fields.at(1);
        l.country = fields.at(2);
        return is;
    }
};

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared, 
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::city_country_container>::char_allocator >
class GeoDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::city_country_container> ; 
        using Keys = vanilla::tagged_tuple<
            typename ipc::data::city_country_entity<Alloc>::city_tag,    std::string, 
            typename ipc::data::city_country_entity<Alloc>::country_tag, std::string
        >;
        using DataVect = typename std::vector<std::shared_ptr <Geo> >;
        using CityCountryTag = typename ipc::data::city_country_entity<Alloc>::unique_city_country_tag;
    public:    
        GeoDataEntity(const Config &config):
            config{config}, cache(config.data().geo_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().geo_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().geo_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().geo_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<Geo>(in), std::istream_iterator<Geo>(), [&](const Geo &geo){
                using namespace boost::algorithm;
                if(!cache.insert(Keys{to_lower_copy(geo.city), to_lower_copy(geo.country)}, geo)) {
                    //LOG(debug) << "Adding city " << geo.city << " country " << geo.country << " failed!";
                }
                else {
                    //LOG(debug) << "Adding city " << geo.city << " country " << geo.country << " done";
                }
                
            });            
        }

        bool retrieve(DataVect &vect, const std::string &city, const std::string &country) {
            bool result = false;
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "geo");
                result = cache.template retrieve<CityCountryTag>(vect, cache.create_ipc_key(city), cache.create_ipc_key(country));
            }
            LOG(debug) << sp->str();
            return result;
        }
    private:
        const Config &config;
        Cache cache;
        
};

#endif /* BIDDER_GEO_HPP */

