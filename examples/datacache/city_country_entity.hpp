/*
 * File:   city_country_entity.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on December 21, 2016, 11:57 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/
 
#ifndef __IPC_DATA_ACCOUNT_ENTITY_HPP__
#define __IPC_DATA_ACCOUNT_ENTITY_HPP__

#include "rtb/datacache/base_entity.hpp"
#include "rtb/datacache/any_str_ops.hpp"

#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <string>
#include <sstream>


namespace ipc { namespace data {
    
    template <typename Alloc>
    struct city_country_entity : base_entity<Alloc>
    {
        using char_string = typename base_entity<Alloc>::char_string;
        using base_type   = base_entity<Alloc>; 
      
        //for tagging in multi_index_container
        struct city_tag {}; // search on city
        struct country_tag {}; // search on country
        struct region_tag {}; // search on region 
        struct unique_city_country_tag {}; //search on city+country or city when using partial search 
       
        city_country_entity( const Alloc & a ) :
        base_entity<Alloc>(a),
        city(a),
        country(a),
        geo_id{}
        {} //ctor END
       
        char_string city;
        char_string country;
        uint32_t geo_id;
 
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            const std::string &key_city     = key.template get<city_tag>() ;
            const std::string &key_country  = key.template get<country_tag>() ;
            city      = char_string(key_city.data(), key_city.size(), base_type::allocator);
            country   = char_string(key_country.data(),  key_country.size(), base_type::allocator) ;
            geo_id    = data.geo_id;
        }
        template<typename Serializable>
        static std::size_t size(const Serializable && data) {
            std::stringstream ss;
            boost::archive::binary_oarchive oarch(ss);
            oarch << std::forward<Serializable>(data) ;
            return base_type::size(std::forward<Serializable>(data)) +
                   sizeof(data.city)                       +
                   sizeof(data.country)                        +
                   ss.str().size() ;
        }
        template<typename Serializable>
        void retrieve(Serializable  & data) const {
            data.city=std::string(city.data(), city.size());
            data.country=std::string(country.data(), country.size());
            data.geo_id=geo_id;
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(city_country_entity &entry) const {
            entry.city=city;
            entry.country=country;
            entry.geo_id=geo_id;
        }
    };


template<typename Alloc>
using city_country_container =
boost::multi_index_container<
    city_country_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<typename city_country_entity<Alloc>::city_tag>,
                BOOST_MULTI_INDEX_MEMBER(city_country_entity<Alloc>,typename city_country_entity<Alloc>::char_string,city),
            ufw::any_str_less<Alloc>
        >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<typename city_country_entity<Alloc>::country_tag>,
                BOOST_MULTI_INDEX_MEMBER(city_country_entity<Alloc>,typename city_country_entity<Alloc>::char_string,country),
            ufw::any_str_less<Alloc>
        >,
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename city_country_entity<Alloc>::unique_city_country_tag>,
            boost::multi_index::composite_key<
                city_country_entity<Alloc>,
                BOOST_MULTI_INDEX_MEMBER(city_country_entity<Alloc>,typename city_country_entity<Alloc>::char_string,city),
                BOOST_MULTI_INDEX_MEMBER(city_country_entity<Alloc>,typename city_country_entity<Alloc>::char_string,country)
            >,
            boost::multi_index::composite_key_compare<
                ufw::any_str_less<Alloc>, ufw::any_str_less<Alloc>
            >
        >
    >,
    boost::interprocess::allocator<city_country_entity<Alloc>,typename Alloc::segment_manager>
> ;

}}
 
#endif     /* __IPC_DATA_ACCOUNT_ENTITY_HPP__  */


