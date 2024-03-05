/* 
 * File:   domain.hpp
 *
 */

#pragma once
#ifndef DOMAIN_HPP
#define DOMAIN_HPP

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
#include "rtb/common/uri_grammar.hpp"
#include "core/tagged_tuple.hpp"
#include "../bidder_experimental/config.hpp"

struct Domain {
    using dom_id_t = uint32_t;
    static constexpr dom_id_t invalid_dom_id = -1u;

    std::string name;
    dom_id_t dom_id{invalid_dom_id};

    Domain() = default;
    Domain(std::string url, dom_id_t ref_id) : name{std::move(url)}, dom_id{ref_id}
    {}
        
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<Domain> &dom) {
        os << *dom;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const Domain &value)  {
        os << value.dom_id << "|"
           << value.name
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, Domain &domain) {
        std::string record;
        if ( !std::getline(is, record) ){
            return is;
        }
        std::vector<std::string> fields;
        boost::split(fields, record, boost::is_any_of("\t"), boost::token_compress_on);
        if(fields.size() < 2) {
            return is;
        }
        domain.name = fields.at(0);
        domain.dom_id = atol(fields.at(1).c_str());
        return is;
    }
};


namespace ipc { namespace data {


    template <typename Alloc>
    struct domain_entity {
        using char_string =  boost::interprocess::basic_string<char, std::char_traits<char>, Alloc>;

        //for tagging in multi_index_container
        struct name_tag {}; // search on name

        domain_entity( const Alloc & a ) : alloc{a}, name{a}, dom_id{}
        {}

        Alloc alloc;
        char_string name;
        uint32_t dom_id;


        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  &&data)  {
           auto url_value = key.template get<name_tag>();
           name = char_string(url_value.data(), url_value.size(), alloc);
           dom_id = data.dom_id;
        }

        template<typename Serializable>
        void retrieve(Serializable  & data) const {
           data.name = std::move(std::string(name.data(), name.length()));
           data.dom_id = dom_id;
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(domain_entity &entry) const {
            entry.name=name;
            entry.dom_id=dom_id;
        }
    };

    template<typename Alloc>
    using domain_container =
    boost::multi_index_container<
        domain_entity<Alloc>,
        boost::multi_index::indexed_by<
            boost::multi_index::ordered_unique<
                  boost::multi_index::tag<typename domain_entity<Alloc>::name_tag>,
                  boost::multi_index::composite_key<
                      domain_entity<Alloc>,
                      BOOST_MULTI_INDEX_MEMBER(domain_entity<Alloc>,typename domain_entity<Alloc>::char_string,name),
                      BOOST_MULTI_INDEX_MEMBER(domain_entity<Alloc>,uint32_t,dom_id)
                  >,
                  boost::multi_index::composite_key_compare<
                      ufw::any_str_less<Alloc> , std::less<uint32_t>
                  >
            >
        >,
        boost::interprocess::allocator<Domain,typename Alloc::segment_manager>
    > ;
}}

template <typename Config = vanilla::config::config<ico_bidder_config_data>,
          typename Memory = typename mpclmi::ipc::Shared, 
          typename Alloc = typename datacache::entity_cache<Memory, ipc::data::domain_container>::char_allocator >
class DomainEntity {
        using Cache = datacache::entity_cache<Memory, ipc::data::domain_container> ;
        using NameTag = typename ipc::data::domain_entity<Alloc>::name_tag;
        using Keys = vanilla::tagged_tuple<NameTag, std::string>;
    public:
        DomainEntity(const Config &config):
            config{config}, cache(config.data().domain_ipc_name)
        {}
        void load() noexcept(false) {
            std::ifstream in{config.data().domain_source};
            if (!in) {
                throw std::runtime_error(std::string("could not open file ") + config.data().domain_source + " exiting...");
            }
            LOG(debug) << "File opened " << config.data().domain_source;
            cache.clear();
            
            std::for_each(std::istream_iterator<Domain>(in), std::istream_iterator<Domain>(), [&](const Domain &domain){
                using namespace boost::algorithm;
                if(!cache.insert(Keys{to_lower_copy(domain.name)}, domain).second) {
                    LOG(debug) << "Adding name " << domain.name << " dom_id " << domain.dom_id << " failed!";
                }
                else {
                    LOG(debug) << "Adding name " << domain.name << " dom_id " << domain.dom_id << " done!";
                }
                
            });
            LOG(info) << "Items loaded " << this->cache.get_size();
        }

        bool retrieve(Domain &domain, const std::string &url) {
            boost::network::uri::detail::uri_parts<std::string::const_iterator> parts;
            boost::network::uri::detail::parse(url.cbegin(), url.cend(), parts);
            if (parts.hier_part.host) {
                std::string host(parts.hier_part.host->begin(), parts.hier_part.host->end());
                return cache.template retrieve<NameTag>(domain, host);
            }
            return false;
        }

    private:
        const Config &config;
        Cache cache;
        
};

#endif /* DOMAIN_HPP */

