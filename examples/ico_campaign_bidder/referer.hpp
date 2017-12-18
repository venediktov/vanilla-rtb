/* 
 * File:   referer.hpp
 *
 */

#pragma once
#ifndef ICO_REFERER_HPP
#define ICO_REFERER_HPP

#include <string>
#include <cstdint>
#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "core/tagged_tuple.hpp"
#include "config.hpp"

struct Referer {
    uint32_t ref_id;
    std::string url;
    std::string country;
    std::string record;
    
    Referer(uint32_t ref_id, std::string url) : 
        geo_id{geo_id}, url{std::move(url)}, record{}
    {}
        
    Referer():
        ref_id{}, record{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<Referer> &referer) {
        os << *referer;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const Referer &value)  {
        os << value.ref_id << "|" 
           << value.url 
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, Referer &l) {
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
class RefererDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::city_country_container> ; 
        using Keys = vanilla::tagged_tuple<
            typename ipc::data::city_country_entity<Alloc>::city_tag,    std::string, 
            typename ipc::data::city_country_entity<Alloc>::country_tag, std::string
        >;
        using DataVect = typename std::vector<std::shared_ptr <Referer> >;
        using CityCountryTag = typename ipc::data::city_country_entity<Alloc>::unique_city_country_tag;
    public:    
        RefererDataEntity(const Config &config):
            config{config}, cache(config.data().geo_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().geo_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().geo_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().geo_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<Referer>(in), std::istream_iterator<Referer>(), [&](const Referer &geo){
                using namespace boost::algorithm;
                if(!cache.insert(Keys{to_lower_copy(geo.city), to_lower_copy(geo.country)}, geo).second) {
                    //LOG(debug) << "Adding city " << geo.city << " country " << geo.country << " failed!";
                }
                else {
                    //LOG(debug) << "Adding city " << geo.city << " country " << geo.country << " done";
                }
                
            });
        }

        bool retrieve(DataVect &vect, const std::string &url) {
            return false;
            // add ability to match referer with negated filter for country , say exclude investors from USA
            //cache.template retrieveIf<RefererTag>(vect, url);
        }

        bool retrieve(Referer &ref, const std::string &url) {
            return true; 
            // add ability to match referer with negated filter for country , say exclude investors from USA
            //cache.template retrieveIf<RefererTag>(ref, url);
        }

    private:
        const Config &config;
        Cache cache;
        
};

#endif /* REFERER_HPP */

