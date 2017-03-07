/* 
 * File:   ad.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:17
 */

#ifndef BIDDER_AD_HPP
#define BIDDER_AD_HPP

#include <typeinfo>
#include <string>
#include <iostream>
#include <cstdint>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "core/tagged_tuple.hpp"
#include "config.hpp"

struct Ad {
    std::string ad_id;
    uint16_t width;
    uint16_t height;
    uint16_t position;
    uint64_t max_bid_micros;
    std::string code;
    std::string record;
    
    Ad(std::string ad_id, uint16_t width, uint16_t height, uint16_t position, uint64_t max_bid_micros, std::string code) : 
        ad_id{std::move(ad_id)}, width{width}, height{height}, position{position}, 
        max_bid_micros{max_bid_micros}, code{std::move(code)}, record{}
    {}
    Ad():
        ad_id{}, width{}, height{}, position{}, max_bid_micros{}, record{}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<Ad> &ad) {
        os <<  *ad;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const Ad &value)  {
        os << value.ad_id << "|" 
           << value.width << "|" 
           << value.height << "|" 
           << value.position << "|" 
           << value.max_bid_micros << "|" 
           << value.code
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, Ad &l) {
        if ( !std::getline(is, l.record) ){
            return is;
        }
        std::vector<std::string> fields;
        boost::split(fields, l.record, boost::is_any_of("\t"), boost::token_compress_on);
        if(fields.size() < 6) {
            return is;
        }
        l.ad_id = fields.at(0); 
        l.width = atoi(fields.at(1).c_str());
        l.height = atoi(fields.at(2).c_str());
        l.position = atoi(fields.at(3).c_str());
        l.max_bid_micros = atol(fields.at(4).c_str());
        l.code = fields.at(5);
        return is;
    }
};

template <typename Config = BidderConfig,
          typename Memory = typename mpclmi::ipc::Shared, 
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::ad_container>::char_allocator >
class AdDataEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::ad_container> ; 
        using Keys = vanilla::tagged_tuple<
            typename ipc::data::ad_entity<Alloc>::width_tag,    uint16_t, 
            typename ipc::data::ad_entity<Alloc>::height_tag,   uint16_t,
            typename ipc::data::ad_entity<Alloc>::ad_id_tag,    std::string
        >;
        using DataVect = std::vector<std::shared_ptr <Ad> >;
        using SizeTag = typename ipc::data::ad_entity<Alloc>::size_ad_id_tag;
    public:
        AdDataEntity(const Config &config):
            config{config}, cache(config.data().ads_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().ads_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().ads_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().ads_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<Ad>(in), std::istream_iterator<Ad>(), [&](const Ad &ad){
                cache.insert(Keys{ad.width, ad.height, ad.ad_id}, ad);
            });            
        }
        template <typename ...Args>
        bool retrieve(DataVect &vect, Args && ...args) {
            bool result = false;
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "ad");
                result = cache.retrieve<SizeTag>(vect, std::forward<Args>(args)...);
            }
            LOG(debug) << sp->str();
            return result;
        }
    private:
        const Config &config;
        Cache cache;
        
};

#endif /* BIDDER_AD_HPP */

