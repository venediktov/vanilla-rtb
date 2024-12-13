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
 
#pragma once

#include <algorithm>
#include <type_traits>
#include <concepts>
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
#include <boost/core/demangle.hpp>
#include <utility>
#include "rtb/core/core.hpp"
#include "rtb/common/concepts.hpp"

#include <boost/throw_exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/stacktrace.hpp>
#include <stdexcept>

// Define an error_info tag for stacktrace

typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;

/** To print stack trace use *boost::get_error_info<traced>(e) after catching e */

template <typename E>
[[noreturn]] void throw_with_stacktrace(E const& e) {
    throw boost::enable_error_info(e)
        << traced(boost::stacktrace::stacktrace());
}

#define VAV_REQUIRE(cond)                                                                                              \
    do {                                                                                                               \
        if (!(cond)) [[unlikely]]                                                                                      \
            throw_with_stacktrace(std::runtime_error("VAV_REQUIRE assertion failed: " #cond));                         \
    } while (false)

namespace bip = boost::interprocess;

namespace datacache {
   
template<typename Index , typename Arg>
auto  find(const Index & idx , Arg && arg) -> decltype(idx.find(arg)) {
    return idx.find(std::forward<Arg>(arg));
}

template<typename Index , typename ...Args>
auto  find(const Index & idx , Args && ...args) -> decltype(idx.find(boost::make_tuple(std::forward<Args>(args)...))) {
    return idx.find(boost::make_tuple(std::forward<Args>(args)...));
}

template<typename Index , typename Arg>
auto  equal_range(const Index & idx , Arg && arg) -> decltype(idx.equal_range(arg)) {
    return idx.equal_range(std::forward<Arg>(arg));
}

template<typename Index , typename ...Args>
auto  equal_range(const Index & idx , Args && ...args) -> decltype(idx.equal_range(boost::make_tuple(std::forward<Args>(args)...))) {
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

inline constexpr size_t ENTITY_CACHE_DEFAULT_MEMORY_SIZE = 67108864 /* 64 MB*/;
inline constexpr size_t ENTITY_CACHE_DEFAULT_MEMORY_GROW_INCREMENT = 67108864 /* 64 MB*/;
template<typename Memory, template <class,class...> class Container, typename ...T>
class entity_cache
{
public:
    using bad_alloc_exception_t  =  boost::interprocess::bad_alloc;
    using segment_manager_t = typename Memory::segment_manager_t;
    using segment_t = typename Memory::segment_t;
    using segment_ptr_t =  boost::scoped_ptr<segment_t>;
    using char_allocator = boost::interprocess::allocator<char, segment_manager_t>;
    using char_string = boost::interprocess::basic_string<char, std::char_traits<char>, char_allocator>;
    using Container_t = Container<char_allocator,T...>;
    using Data_t = typename Container_t::value_type;

    /** Can set memory_grow_increment=0 to prevent cache from growing */
    explicit entity_cache(std::string name, size_t const memory_size = ENTITY_CACHE_DEFAULT_MEMORY_SIZE,
                          size_t const memory_grow_increment = ENTITY_CACHE_DEFAULT_MEMORY_GROW_INCREMENT)
        : _segment_ptr(), _container_ptr(), _store_name(), _cache_name(std::move(name)),
          _named_mutex(bip::open_or_create, (_cache_name + "_mutex").c_str()),
          _memory_grow_increment{memory_grow_increment} {
        // TODO: add to ctor to switch between mmap and shm
        // TODO: maybe needs bip::scoped_lock to lock for other processes calling  grow_memory
        std::string data_base_dir = "/tmp/CACHE";
        _store_name = Memory::convert_base_dir(data_base_dir) + _cache_name;
        _segment_ptr.reset(Memory::open_or_create_segment(_store_name.c_str(), memory_size));
        _container_ptr = _segment_ptr->template find_or_construct<Container_t>(_cache_name.c_str())(
            typename Container_t::ctor_args_list(),
            typename Container_t::allocator_type(_segment_ptr->get_segment_manager()));
    }

    template <std::invocable F>
    auto read(F&& f) {
        auto guard = bip::sharable_lock{_named_mutex};
        return f();
    }

    template <std::invocable F>
    auto modify(F&& f) {
        auto guard = bip::scoped_lock{_named_mutex};
        return f();
    }

    void clear() {
        bip::scoped_lock guard(_named_mutex) ;
        _container_ptr->clear();
    }
   
    template<typename Tag, typename Key, typename Serializable, typename Arg>
    bool update( Key && key, Serializable && data, Arg&& arg) {
        bip::scoped_lock guard(_named_mutex) ;
        bool is_success {false};
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(std::forward<Arg>(arg));
        while (p.first != p.second) {
            is_success |= retried_with_grow("update_data", [&, this] {
                return update_data(std::forward<Key>(key),std::forward<Serializable>(data),index,p.first++);
            });
        }
        return is_success;
    }
 
    template<typename Tag, typename Key, typename Serializable, typename ...Args>
    bool update( Key && key, Serializable && data, Args&& ...args) {
        bip::scoped_lock guard(_named_mutex) ;
        bool is_success {false};
        //Memory::attach([this](){attach();}); // reattach to newly created
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(boost::make_tuple(std::forward<Args>(args)...));
        while ( p.first != p.second ) {
            is_success |= retried_with_grow("update_data", [&, this] {
                return update_data(std::forward<Key>(key),std::forward<Serializable>(data),index,p.first++);
            });
        }
        return is_success;
    }

    template<typename Tag, typename Functor, typename Arg>
    bool update( Functor && func, Arg&& arg) {
        bip::scoped_lock guard(_named_mutex) ;
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(std::forward<Arg>(arg));
        return modify(index, p.first, p.second, std::forward<Functor>(func));
    }

    template<typename Tag, typename Functor, typename ...Args>
    bool update( Functor && func, Args&& ...args) {
        bip::scoped_lock guard(_named_mutex) ;
        auto &index = _container_ptr->template get<Tag>();
        auto p = index.equal_range(boost::make_tuple(std::forward<Args>(args)...));
        return modify(index, p.first, p.second, std::forward<Functor>(func));
    }

    template<typename Key, typename Serializable>
    auto insert( Key && key, Serializable &&data) {
        bip::scoped_lock guard(_named_mutex) ;
        return insert_unsafe(std::forward<Key>(key), std::forward<Serializable>(data));
    }

    template<typename Key, typename Serializable>
    auto insert_unsafe( Key && key, Serializable &&data) {
        return retried_with_grow("insert_data", [&, this] {
            return insert_data(std::forward<Key>(key), std::forward<Serializable>(data));
        });
    }

    template<typename Tag, typename Serializable, typename ...Args>
    bool retrieve(Serializable &entry, Args&& ...args) {
        bip::sharable_lock guard(_named_mutex);
        return retriever<Tag,Serializable>()(*_container_ptr,entry,std::forward<Args>(args)...);
    }

    template<typename Tag, typename ...Args>
    auto retrieve_raw(Args&& ...args) {
        bip::sharable_lock guard(_named_mutex);
        auto &idx = _container_ptr->template get<Tag>();
        return equal_range(idx, std::forward<Args>(args)...);
    }
    
    template<typename Serializable>
    bool retrieve(std::vector<std::shared_ptr<Serializable>> &entries) {
        bip::sharable_lock guard(_named_mutex);
        auto p = std::make_pair(_container_ptr->begin(), _container_ptr->end());
        std::transform ( p.first, p.second, std::back_inserter(entries), [] ( const Data_t &data ) {
            std::shared_ptr<Serializable> impl_ptr { std::make_shared<Serializable>() } ;
            data.retrieve(*impl_ptr) ;
            return impl_ptr;
        });
        return !entries.empty();
    }

    template<typename Tag, typename ...Args>
    auto remove(Args&& ...args) {
        bip::scoped_lock guard(_named_mutex);
        return remove_unsafe<Tag>(std::forward<Args>(args)...);
    }

    template<typename Tag, typename ...Args>
    void remove_unsafe(Args&& ...args) {
        auto p = _container_ptr->template get<Tag>().equal_range(boost::make_tuple(std::forward<Args>(args)...));
        _container_ptr->erase(p.first, p.second);
    }

    template<typename Tag, typename Arg>
    auto remove(Arg && arg) {
        bip::scoped_lock guard(_named_mutex);
        return remove_unsafe<Tag>(std::forward<Arg>(arg));
    }

    template<typename Tag, typename Arg>
    void remove_unsafe(Arg && arg) {
        auto p = _container_ptr->template get<Tag>().equal_range(std::forward<Arg>(arg));
        _container_ptr->erase(p.first, p.second);
    }

    char_string create_ipc_key(const std::string &key)  const {
        return retried_with_grow("create_ipc_key", [&, this] {
            return char_string{key.data(), key.size(), _segment_ptr->get_segment_manager()};
        });
    }

    size_t get_size() const {
        bip::sharable_lock guard(_named_mutex) ;
        return _container_ptr->size();
    }
private:
    auto retried_with_grow([[maybe_unused]] std::string_view op_name, auto&& op) {
        try {
            return op();
        } catch ([[maybe_unused]] bad_alloc_exception_t const& e) {
#ifndef NDEBUG
            LOG(debug) << boost::core::demangle(typeid(*this).name()) << " " << op_name
                       << " failed , MEMORY AVAILABLE=" << _segment_ptr->get_free_memory();
#endif
            if (_memory_grow_increment && !grow_memory(_memory_grow_increment)) [[unlikely]] {
                throw;
            }

            return op();
        }
    }

    void attach() const {
        if constexpr (!std::is_same_v<segment_t, boost::interprocess::managed_heap_memory>) {
            _segment_ptr.reset(new segment_t(bip::open_only, _store_name.c_str()));
        }
        _container_ptr = _segment_ptr->template find_or_construct<Container_t>(_cache_name.c_str())(
            typename Container_t::ctor_args_list(),
            typename Container_t::allocator_type(_segment_ptr->get_segment_manager()));
    }

    bool grow_memory(size_t size) const {
        VAV_REQUIRE(_memory_grow_increment > 0);

        bool success = false;
        try {
            if constexpr (!std::is_same_v<segment_t, boost::interprocess::managed_heap_memory>) {
                _segment_ptr.reset();
            }
            Memory::grow(_segment_ptr, _store_name.c_str(), size);
            success = true;
        } catch (bad_alloc_exception_t const& e) {
            LOG(debug) << boost::core::demangle(typeid(*this).name()) << " failed to grow " << e.what()
                       << ":free mem=" << _segment_ptr->get_free_memory();
        }
        Memory::attach([this]() { attach(); }); // reattach to newly created
        return success;
    }
 
    template<typename Key, typename Serializable>
    auto insert_data(Key && key, Serializable &&data) {
        if (_memory_grow_increment > 0) {
            Memory::attach([this]{ attach(); });
        }

        Data_t item(_segment_ptr->get_segment_manager());
        item.store(std::forward<Key>(key), std::forward<Serializable>(data));
        return _container_ptr->insert(item);
    }
 
    template<typename Key, typename Serializable, typename Index, typename Iterator>
    bool update_data(Key && key, Serializable && data, Index &index, Iterator itr) {
        Data_t item(_segment_ptr->get_segment_manager());
        item.store(std::forward<Key>(key), std::forward<Serializable>(data));
        return index.modify(itr,item) ;
    }

    template<typename Index, typename Iterator , typename Functor, typename ...Args>
    auto modify ( Index & index, Iterator first , Iterator last, Functor && func) {
        bool is_success {false};
        while ( first != last ) {
            is_success |= retried_with_grow("modify", [&] {
                return index.modify(first++, std::forward<Functor>(func));
            });
        }
        return is_success;
    }
 
    mutable boost::scoped_ptr<segment_t> _segment_ptr;
    mutable Container_t* _container_ptr{nullptr};
    std::string _store_name;
    std::string _cache_name;
    mutable boost::interprocess::named_upgradable_mutex _named_mutex;
    size_t _memory_grow_increment{0};
};
 
}

