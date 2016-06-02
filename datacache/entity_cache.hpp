/*
 * File:   entity_cache.hpp
 * Author: Vladimir Venediktov
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on April 29, 2016, 12:40 PM
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
*
*/
 
#ifndef __DATACACHE_ENTITY_CACHE_HPP__
#define __DATACACHE_ENTITY_CACHE_HPP__

#include <algorithm>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <memory>
 
#include <boost/version.hpp>
#if BOOST_VERSION > 105700
#include <boost/core/demangle.hpp>
#else
#include <cxxabi.h>
namespace boost { namespace core {
std::string demangle(const char* name) {
   int status=-4; int i = BOOST_VERSION ;
   std::unique_ptr<char, void(*)(void *)> res {
       abi::__cxa_demangle(name, NULL, NULL, &status),
       std::free
   } ;
   return (status==0) ? res.get() : name ;
} 
}}
#endif

#define LOG(x) BOOST_LOG_TRIVIAL(x)

namespace {
    namespace bip = boost::interprocess ;
}
 
namespace datacache {
   
template<typename Memory, template <class> class Container>
class entity_cache
{
public:
    using bad_alloc_exception_t  =  boost::interprocess::bad_alloc ;
    using segment_manager_t = typename Memory::segment_manager_t ;
    using segment_t = typename Memory::segment_t ;
    using segment_ptr_t =  boost::scoped_ptr<segment_t>  ;
    using char_allocator = boost::interprocess::allocator<char, segment_manager_t>  ;
    using char_string = boost::interprocess::basic_string<char, std::char_traits<char>, char_allocator>   ;
    using Container_t = Container<char_allocator> ;
    using Data_t = typename Container_t::value_type;
       
entity_cache(const std::string &name) : _segment_ptr(), _container_ptr(), _store_name(), _cache_name(name),
_named_mutex(bip::open_or_create, (_cache_name + "_mutex").c_str()) {
//TODO: add to ctor to switch between mmap and shm
std::string data_base_dir = "/tmp/CACHE" ;
_store_name =  Memory::convert_base_dir(data_base_dir) + _cache_name;
_segment_ptr.reset(new segment_t(bip::open_or_create, _store_name.c_str(), MEMORY_SIZE) ) ;
_container_ptr = _segment_ptr->template find_or_construct<Container_t>( _cache_name.c_str() )
    (typename Container_t::ctor_args_list() , typename Container_t::allocator_type(_segment_ptr->get_segment_manager()));
 
}
    void clear() {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        _container_ptr->clear() ;
    }
   
    template<typename Tag, typename Serializable, typename Arg>
    bool update( const Serializable &data, Arg&& arg) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        bool is_success {false};
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(std::forward<Arg>(arg));      
        while ( p.first != p.second ) {
            try {
              is_success |= update_data(data,index,p.first++);
            } catch (const bad_alloc_exception_t &e) {
              LOG(debug) << boost::core::demangle(typeid(*this).name())
              << " data was not updated , MEMORY AVAILABLE="
              <<  _segment_ptr->get_free_memory() ;
              grow_memory(MEMORY_SIZE);
              is_success |= update_data(data,index,p.first++);
            }
        }
        return is_success;
    }
 
    template<typename Tag, typename Serializable, typename ...Args>
    bool update( const Serializable &data, Args&& ...args) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        bool is_success {false};
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(boost::make_tuple(std::forward<Args>(args)...));
         while ( p.first != p.second ) {
            try {
              is_success |= update_data(data,index,p.first++);
            } catch (const bad_alloc_exception_t &e) {
              LOG(debug) << boost::core::demangle(typeid(*this).name())
              << " data was not updated , MEMORY AVAILABLE="
              <<  _segment_ptr->get_free_memory() ;
              grow_memory(MEMORY_SIZE);
              is_success |= update_data(data,index,p.first++);
            }
        }
        return is_success;
    }
 
    template<typename Serializable>
    bool insert( const Serializable &data) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        bool is_success {false};
        try {
            is_success = insert_data(data);
        } catch (const bad_alloc_exception_t &e) {
            LOG(debug) << boost::core::demangle(typeid(*this).name())
            << " data was not inserted , MEMORY AVAILABLE="
            <<  _segment_ptr->get_free_memory(); 
            grow_memory(MEMORY_SIZE);
            is_success = insert_data(data);
        }
 
        return is_success;
    }
   
    template<typename Serializable>
    bool insert( const std::vector<Serializable> &data) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        bool is_success {false};
        std::size_t n {data.size()} ;
        for ( const auto &item : data) {
            try {
                if ( insert_data(data) ) { --n; }
            } catch (const bad_alloc_exception_t &e) {
                LOG(debug) << boost::core::demangle(typeid(*this).name())
                << " data was not inserted , MEMORY AVAILABLE="
                <<  _segment_ptr->get_free_memory(); 
                grow_memory(MEMORY_SIZE);
                if ( insert_data(data) ) { --n; }
            }
        }
        return !n;
    }
       
    template<typename Tag, typename Serializable, typename Arg>
    bool retrieve(std::vector<std::shared_ptr<Serializable>> &entries, Arg && arg) {
        bip::sharable_lock<bip::named_upgradable_mutex> guard(_named_mutex);
        bool is_found = false;
        auto p = _container_ptr->template get<Tag>().equal_range(std::forward<Arg>(arg));
        std::transform ( p.first, p.second, std::back_inserter(entries), [] ( const Data_t &data ) {
            std::shared_ptr<Serializable> impl_ptr { std::make_shared<Serializable>() } ;
            data.retrieve(*impl_ptr) ;
            return impl_ptr;
        });
        return !entries.empty();
    }
      
    template<typename Tag, typename Serializable, typename ...Args>
    bool retrieve(std::vector<std::shared_ptr<Serializable>> &entries, Args&& ...args) {
        bip::sharable_lock<bip::named_upgradable_mutex> guard(_named_mutex);
        bool is_found = false;
        auto p = _container_ptr->template get<Tag>().equal_range(boost::make_tuple(std::forward<Args>(args)...));
        std::transform ( p.first, p.second, std::back_inserter(entries), [] ( const Data_t &data ) {
            std::shared_ptr<Serializable> impl_ptr { std::make_shared<Serializable>() } ;
            data.retrieve(*impl_ptr) ;
            return impl_ptr;
        });
        return !entries.empty();
    }
 
 
    template<typename Serializable>
    bool retrieve(std::vector<std::shared_ptr<Serializable>> &entries) {
        bip::sharable_lock<bip::named_upgradable_mutex> guard(_named_mutex);
        bool is_found = false;
        auto p = std::make_pair(_container_ptr->begin(), _container_ptr->end());
        std::transform ( p.first, p.second, std::back_inserter(entries), [] ( const Data_t &data ) {
            std::shared_ptr<Serializable> impl_ptr { std::make_shared<Serializable>() } ;
            data.retrieve(*impl_ptr) ;
            return impl_ptr;
        });
        return !entries.empty();
    }
  
   char_string create_ipc_key(const std::string &key)  const {
       try {
           char_string tmp(key.data(), key.size(), _segment_ptr->get_segment_manager()) ;
           return tmp;
       } catch ( const  bad_alloc_exception_t &e ) {
           LOG(debug) << boost::core::demangle(typeid(*this).name())
           << " create_ipc_key failed , MEMORY AVAILABLE="
           <<  _segment_ptr->get_free_memory(); 
           grow_memory(MEMORY_SIZE) ;
           char_string tmp(key.data(), key.size(), _segment_ptr->get_segment_manager()) ;
           return tmp;
       }
   }
private:
    void attach() const {
    _segment_ptr.reset(new segment_t(bip::open_only,_store_name.c_str()) ) ;
    _container_ptr = _segment_ptr->template find_or_construct<Container_t>(_cache_name.c_str())
        (typename Container_t::ctor_args_list(), typename Container_t::allocator_type(_segment_ptr->get_segment_manager()));
    }
 
    void grow_memory(size_t size) const {
        try {
          _segment_ptr.reset() ;
          segment_t::grow(_store_name.c_str(), size) ;
        } catch ( const  bad_alloc_exception_t &e ) {
            LOG(debug) << boost::core::demangle(typeid(*this).name())       
            << " failed to grow " << e.what() << ":free mem=" << _segment_ptr->get_free_memory() ;
        }
        attach() ; // reattach to newly created
    }
 
    template<typename Serializable>
    bool insert_data(const  Serializable &data) {
        attach();
        Data_t item(_segment_ptr->get_segment_manager());
        item.store(data);
        return _container_ptr->insert(item).second;
    }
 
    template<typename Serializable, typename Index, typename Iterator>
    bool update_data(const  Serializable &data, Index &index, Iterator itr) {
        attach();
        Data_t item(_segment_ptr->get_segment_manager());
        item.store(data);
        return index.modify(itr,item) ;
    }
 
    mutable boost::scoped_ptr<segment_t> _segment_ptr;
    mutable Container_t  *_container_ptr ;
    std::string _store_name ;
    std::string _cache_name ;
    boost::interprocess::named_upgradable_mutex _named_mutex;
    static const size_t MEMORY_SIZE = 67108864 ; //64M
 
};
 
}
 
 
#endif /* __DATACACHE_ENTITY_CACHE_HPP__ */


