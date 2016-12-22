/*
 * File:   account_entity.hpp
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
 
#include <string>
#include <sstream>
#include <boost/tuple/tuple.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
  
namespace ipc { namespace data {
    
    template <typename Alloc>
    struct account_entity
    {
        using char_string =  boost::interprocess::basic_string<char, std::char_traits<char>, Alloc> ;
      
        //for tagging in multi_index_container
        struct parent_account_tag {}; // search on parent_account
        struct child_account_tag {}; // search on child_account
        struct unique_account_tag {}; // search on parent_account+child_account or parent_account
       
        account_entity( const Alloc & a ) :
        _allocator(a),
        parent_account(a),
        child_account(a),
        blob(a)
        {} //ctor END
       
        Alloc _allocator ;
        char_string parent_account;
        char_string child_account;
        char_string blob;
 
        template<typename Key, typename Serializable>
        void store(Key && key, Serializable  && data)  {
            std::stringstream ss;
            boost::archive::binary_oarchive oarch(ss);
            oarch << data ;
            std::string blob_str = std::move(ss.str()) ;
            blob = char_string(blob_str.data(), blob_str.length(), _allocator) ;
            //Store keys
            const std::string &key_parent = key.template get<parent_account_tag>() ;
            const std::string &key_child  = key.template get<child_account_tag>() ;
            parent_account  = char_string(key_parent.data(), key_parent.size(), _allocator);
            child_account   = char_string(key_child.data(),  key_child.size(), _allocator) ;
        }
        template<typename Serializable>
        static std::size_t size(const Serializable &data) {
            std::stringstream ss;
            boost::archive::binary_oarchive oarch(ss);
            oarch << data ;
            return sizeof(data._allocator)              +
                   sizeof(data.parent_account)          +
                   sizeof(data.child_account)           +
                   ss.str().size() ;
        }
        template<typename Serializable>
        void retrieve(Serializable  &data) const {           
            std::stringstream ss (std::string(blob.data(),blob.length()));
            boost::archive::binary_iarchive iarch(ss);
            iarch >> data;
        }
        //needed for ability to update after matching by calling index.modify(itr,entry)
        void operator()(account_entity &entry) const {
            entry.parent_account=parent_account;
            entry.child_account=child_account;
            entry.blob=blob;
        }
    };
   
 
 
template<typename Alloc>
using account_container =
boost::multi_index_container<
    account_entity<Alloc>,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<typename account_entity<Alloc>::parent_account_tag>,
                BOOST_MULTI_INDEX_MEMBER(account_entity<Alloc>,typename account_entity<Alloc>::char_string,parent_account)
        >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<typename account_entity<Alloc>::child_account_tag>,
                BOOST_MULTI_INDEX_MEMBER(account_entity<Alloc>,typename account_entity<Alloc>::char_string,child_account)
        >,
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<typename account_entity<Alloc>::unique_account_tag>,
            boost::multi_index::composite_key<
                account_entity<Alloc>,
                BOOST_MULTI_INDEX_MEMBER(account_entity<Alloc>,typename account_entity<Alloc>::char_string,parent_account),
                BOOST_MULTI_INDEX_MEMBER(account_entity<Alloc>,typename account_entity<Alloc>::char_string,child_account)
            >
        >
    >,
    boost::interprocess::allocator<account_entity<Alloc>,typename Alloc::segment_manager>
> ;
  
    
}}
 
#endif     /* __IPC_DATA_ACCOUNT_ENTITY_HPP__  */


