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
#include <memory>
 
#include <boost/version.hpp>
#if BOOST_VERSION >= 105600
#include <boost/core/demangle.hpp>
#elif defined(__GNUC__)
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
#else 
#error "Please upgrade version of Boost to 1.56 or higher" 
#endif

#include "rtb/core/core.hpp"

namespace {
    namespace bip = boost::interprocess ;
}
 
namespace datacache {
   
template<typename Index , typename Arg>
auto  find(const Index & idx , Arg && arg) -> decltype(idx.find(arg)) {
    return idx.find(std::forward<Arg>(arg));
}

template<typename Index , typename ...Args>
auto  find(const Index & idx , Args && ...args) -> decltype(idx.find(std::forward<Args>(args)...)) {
    return idx.find(boost::make_tuple(std::forward<Args>(args)...));
}

template<typename Index , typename Arg>
auto  equal_range(const Index & idx , Arg && arg) -> decltype(idx.equal_range(arg)) {
    return idx.equal_range(std::forward<Arg>(arg));
}

template<typename Index , typename ...Args>
auto  equal_range(const Index & idx , Args && ...args) -> decltype(idx.equal_range(std::forward<Args>(args)...)) {
    return idx.equal_range(boost::make_tuple(std::forward<Args>(args)...));
}


template<typename Tag, typename Serializable>
struct retriever {
    template<typename Container, typename ...Args>
    bool operator()(const Container & c, Serializable & entry, Args && ...args) {
        auto &idx = c.template get<Tag>();
        auto p = find(idx, std::forward<Args>(args)...);
        bool is_found = p != idx.end();
        if ( is_found ) {
            p->retrieve(entry);
        }
        return is_found;
    }
};

template<typename Tag, typename Serializable>
struct retriever<Tag,std::vector<std::shared_ptr<Serializable>>> {
    template<typename Container, typename ...Args>
    bool operator()(const Container & c, std::vector<std::shared_ptr<Serializable>> &entries, Args && ...args) {
        auto &idx = c.template get<Tag>();
        auto p = equal_range(idx, std::forward<Args>(args)...);
        std::transform ( p.first, p.second, std::back_inserter(entries), [] ( const auto &data ) {
            std::shared_ptr<Serializable> impl_ptr { std::make_shared<Serializable>() } ;
            data.retrieve(*impl_ptr) ;
            return impl_ptr;
        });
        return !entries.empty();
    }
};
 
template<typename Memory, template <class> class Container, size_t MEMORY_SIZE = 67108864 >
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
       
    entity_cache(const std::string &name) : 
        _segment_ptr(), _container_ptr(), _store_name(), _cache_name(name), _named_mutex(bip::open_or_create, (_cache_name + "_mutex").c_str()) {
        //TODO: add to ctor to switch between mmap and shm
        //TODO: maybe needs bip::scoped_lock to lock for other processes calling  grow_memory    
        std::string data_base_dir = "/tmp/CACHE" ;
        _store_name =  Memory::convert_base_dir(data_base_dir) + _cache_name;
        _segment_ptr.reset(Memory::open_or_create_segment(_store_name.c_str(), MEMORY_SIZE) ) ;
        _container_ptr = _segment_ptr->template find_or_construct<Container_t>( _cache_name.c_str() )
        (typename Container_t::ctor_args_list() , typename Container_t::allocator_type(_segment_ptr->get_segment_manager()));
    }
    
    void clear() {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        _container_ptr->clear() ;
    }
   
    template<typename Tag, typename Key, typename Serializable, typename Arg>
    bool update( Key && key, Serializable && data, Arg&& arg) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        bool is_success {false};
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(std::forward<Arg>(arg));
        while ( p.first != p.second ) {
            try {
              is_success |= update_data(std::forward<Key>(key),std::forward<Serializable>(data),index,p.first++);
            } catch (const bad_alloc_exception_t &e) {
              LOG(debug) << boost::core::demangle(typeid(*this).name())
              << " data was not updated , MEMORY AVAILABLE="
              <<  _segment_ptr->get_free_memory() ;
              grow_memory(MEMORY_SIZE);
              is_success |= update_data(std::forward<Key>(key),std::forward<Serializable>(data),index,p.first++);
            }
        }
        return is_success;
    }
 
    template<typename Tag, typename Key, typename Serializable, typename ...Args>
    bool update( Key && key, Serializable && data, Args&& ...args) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        bool is_success {false};
        //Memory::attach([this](){attach();}); // reattach to newly created
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(boost::make_tuple(std::forward<Args>(args)...));
         while ( p.first != p.second ) {
            try {
              is_success |= update_data(std::forward<Key>(key),std::forward<Serializable>(data),index,p.first++);
            } catch (const bad_alloc_exception_t &e) {
              LOG(debug) << boost::core::demangle(typeid(*this).name())
              << " data was not updated , MEMORY AVAILABLE="
              <<  _segment_ptr->get_free_memory() ;
              grow_memory(MEMORY_SIZE);
              is_success |= update_data(std::forward<Key>(key),std::forward<Serializable>(data),index,p.first++);
            }
        }
        return is_success;
    }
 
    template<typename Key, typename Serializable>
    bool insert( Key && key, Serializable &&data) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex) ;
        bool is_success {false};
        try {
            is_success = insert_data(std::forward<Key>(key), std::forward<Serializable>(data));
        } catch (const bad_alloc_exception_t &e) {
            LOG(debug) << boost::core::demangle(typeid(*this).name())
            << " data was not inserted , MEMORY AVAILABLE="
            <<  _segment_ptr->get_free_memory(); 
            grow_memory(MEMORY_SIZE);
            is_success = insert_data(std::forward<Key>(key), std::forward<Serializable>(data));
        }
 
        return is_success;
    }
  
/***************** 
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
*******************/
    
    template<typename Tag, typename Serializable, typename ...Args>
    bool retrieve(Serializable &entry, Args&& ...args) {
        bip::sharable_lock<bip::named_upgradable_mutex> guard(_named_mutex);
        return retriever<Tag,Serializable>()(*_container_ptr,entry,std::forward<Args>(args)...);
    }

    template<typename Tag, typename ...Args>
    auto retrieve_raw(Args&& ...args) {
        bip::sharable_lock<bip::named_upgradable_mutex> guard(_named_mutex);
        auto &idx = _container_ptr->template get<Tag>();
        return equal_range(idx, std::forward<Args>(args)...);
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

    template<typename Tag, typename ...Args>
    void remove(Args&& ...args) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex);
        auto p = _container_ptr->template get<Tag>().equal_range(boost::make_tuple(std::forward<Args>(args)...));
        _container_ptr->erase(p.first, p.second);
    }
    
    template<typename Tag, typename Arg>
    void remove(Arg && arg) {
        bip::scoped_lock<bip::named_upgradable_mutex> guard(_named_mutex);
        auto p = _container_ptr->template get<Tag>().equal_range(std::forward<Arg>(arg));
        _container_ptr->erase(p.first, p.second);
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
          Memory::grow(_segment_ptr, _store_name.c_str(), size) ;
        } catch ( const  bad_alloc_exception_t &e ) {
            LOG(debug) << boost::core::demangle(typeid(*this).name())       
            << " failed to grow " << e.what() << ":free mem=" << _segment_ptr->get_free_memory() ;
        }
        Memory::attach([this](){attach();}); // reattach to newly created
    }
 
    template<typename Key, typename Serializable>
    bool insert_data(Key && key, Serializable &&data) {
        Memory::attach([this](){attach();});
        Data_t item(_segment_ptr->get_segment_manager());
        item.store(std::forward<Key>(key), std::forward<Serializable>(data));
        return _container_ptr->insert(item).second;
    }
 
    template<typename Key, typename Serializable, typename Index, typename Iterator>
    bool update_data(Key && key, Serializable && data, Index &index, Iterator itr) {
        Data_t item(_segment_ptr->get_segment_manager());
        item.store(std::forward<Key>(key), std::forward<Serializable>(data));
        return index.modify(itr,item) ;
    }
 
    mutable boost::scoped_ptr<segment_t> _segment_ptr;
    mutable Container_t  *_container_ptr ;
    std::string _store_name ;
    std::string _cache_name ;
    boost::interprocess::named_upgradable_mutex _named_mutex;
};
 
}
 
 
#endif /* __DATACACHE_ENTITY_CACHE_HPP__ */


