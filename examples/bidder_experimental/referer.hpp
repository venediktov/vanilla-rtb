/* 
 * File:   referer.hpp
 *
 */

#pragma once
#ifndef REFERER_HPP
#define REFERER_HPP

#include <string>
#include <cstdint>
#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include "rtb/datacache/any_str_ops.hpp"
#include "core/tagged_tuple.hpp"
#include "config.hpp"

struct Referer {
    std::string url;
    uint32_t ref_id;

    Referer(std::string url, uint32_t ref_id) : url{std::move(url)}, ref_id{ref_id}
    {}
        
    Referer(): url{}
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
        std::string record;
        if ( !std::getline(is, record) ){
            return is;
        }
        std::vector<std::string> fields;
        boost::split(fields, record, boost::is_any_of("\t"), boost::token_compress_on);
        if(fields.size() < 2) {
            return is;
        }
        l.ref_id = atol(fields.at(0).c_str());
        l.url = fields.at(1);
        return is;
    }
};


namespace ipc { namespace data {


    template <typename Alloc>
    struct referer_entity {
        using char_string =  boost::interprocess::basic_string<char, std::char_traits<char>, Alloc>;

        //for tagging in multi_index_container
        struct url_tag {}; // search on url

        referer_entity( const Alloc & a ) : alloc{a}, url{a}, ref_id{}
        {}

        Alloc alloc;
        char_string url;
        uint32_t ref_id;


        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  &&data)  {
           auto url_value = key.template get<url_tag>();
           url = char_string(url_value.data(), url_value.size(), alloc);
           ref_id = data.ref_id;
        }

        template<typename Serializable>
        void retrieve(Serializable  & data) const {
           data.url = std::move(std::string(url.data(), url.length()));
           data.ref_id = ref_id;
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(referer_entity &entry) const {
            entry.url=url;
            entry.ref_id=ref_id;
        }
    };

    template<typename Alloc>
    using referer_container =
    boost::multi_index_container<
        referer_entity<Alloc>,
        boost::multi_index::indexed_by<
            boost::multi_index::ordered_unique<
                  boost::multi_index::tag<typename referer_entity<Alloc>::url_tag>,
                  boost::multi_index::composite_key<
                      referer_entity<Alloc>,
                      BOOST_MULTI_INDEX_MEMBER(referer_entity<Alloc>,typename referer_entity<Alloc>::char_string,url),
                      BOOST_MULTI_INDEX_MEMBER(referer_entity<Alloc>,uint32_t,ref_id)
                  >,
                  boost::multi_index::composite_key_compare<
                      ufw::any_str_less<Alloc> , std::less<uint32_t>
                  >
            >
        >,
        boost::interprocess::allocator<Referer,typename Alloc::segment_manager>
    > ;
}}

template <typename Config = vanilla::config::config<ico_bidder_config_data>,
          typename Memory = typename mpclmi::ipc::Shared, 
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::referer_container>::char_allocator >
class RefererEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::referer_container> ;
        using UrlTag = typename ipc::data::referer_entity<Alloc>::url_tag;
        using Keys = vanilla::tagged_tuple<UrlTag, std::string>;
    public:
        RefererEntity(const Config &config):
            config{config}, cache(config.data().referer_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().referer_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().referer_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().referer_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<Referer>(in), std::istream_iterator<Referer>(), [&](const Referer &referer){
                using namespace boost::algorithm;
                if(!cache.insert(Keys{to_lower_copy(referer.url)}, referer).second) {
                    LOG(debug) << "Adding url " << referer.url << " ref_id " << referer.ref_id << " failed!";
                }
                else {
                    LOG(debug) << "Adding url " << referer.url << " ref_id " << referer.ref_id << " done!";
                }
                
            });
        }

        bool retrieve(Referer &ref, const std::string &url) {
            return  cache.template retrieve<UrlTag>(ref, url);
        }

    private:
        const Config &config;
        Cache cache;
        
};

#endif /* REFERER_HPP */

