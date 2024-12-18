/* 
 * File:   memory_types.hpp
 * Author: Valdimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on March 19, 2014, 4:02 PM
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

#ifndef __IPC_MEMORY_TYPES_HPP__
#define	__IPC_MEMORY_TYPES_HPP__

#include <boost/interprocess/managed_shared_memory.hpp> //Variant-I , shm_open(), mmap()
#include <boost/interprocess/managed_mapped_file.hpp> //Variant-II , open(), mmap()
#include <boost/interprocess/managed_heap_memory.hpp> // Variant III for heap
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/mem_algo/simple_seq_fit.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace mpclmi::ipc {

/** A variation of the Shared:
 * - simpler memory allocation algorithm suitable for segments storing a single node-based container
 * - memory allocation is not thread safe, relying on external synchronization typical for entity_cache-based containers
 */
struct SharedSeqFitNoLock {
    using char_type = char;
    using mutex_family_type = boost::interprocess::null_mutex_family;
    using memory_algorithm_type = boost::interprocess::simple_seq_fit<mutex_family_type>;
    template <typename T> using index_type = boost::interprocess::iset_index<T>;
    using segment_t = boost::interprocess::basic_managed_shared_memory<char_type, memory_algorithm_type, index_type>;
    using segment_manager_t = boost::interprocess::segment_manager<char, memory_algorithm_type, index_type>;

    static segment_t * open_or_create_segment (const std::string &path, size_t size) {
        return new segment_t(boost::interprocess::open_or_create, path.c_str(), size) ;
    }
    static segment_t * open_segment (const std::string &path) {
        return new segment_t(boost::interprocess::open_only, path.c_str()) ;
    }
    static segment_t * create_segment (const std::string &path, size_t size) {
        return new segment_t(boost::interprocess::create_only, path.c_str(), size) ;
    }
    template <typename MemPtr>
    static void grow(MemPtr &mem_ptr , const std::string &path, size_t size) {
        mem_ptr.reset() ;
        segment_t::grow(path.c_str(), size) ;
        mem_ptr.reset(open_segment(path)) ;
    }
    static std::string convert_base_dir([[maybe_unused]] const std::string &base_dir) {
        return "" ;
    }

    template<typename Function>
    static void attach( Function && f ) {
        f() ;
    }
};


struct Shared {
    typedef boost::interprocess::managed_shared_memory   segment_t;
    typedef boost::interprocess::managed_shared_memory::segment_manager  segment_manager_t;
    typedef boost::shared_mutex lock_t ;
    typedef boost::unique_lock<lock_t>  scoped_exclusive_locker_t;
    typedef boost::shared_lock<lock_t>  scoped_shared_locker_t;
    static segment_t * open_or_create_segment (const std::string &path, size_t size) {
        return new segment_t(boost::interprocess::open_or_create, path.c_str(), size) ;
    }
    static segment_t * open_segment (const std::string &path) {
        return new segment_t(boost::interprocess::open_only, path.c_str()) ; 
    }
    static segment_t * create_segment (const std::string &path, size_t size) {
        return new segment_t(boost::interprocess::create_only, path.c_str(), size) ; 
    }
    template <typename MemPtr>
    static void grow(MemPtr &mem_ptr , const std::string &path, size_t size) {
        mem_ptr.reset() ;
        segment_t::grow(path.c_str(), size) ;
        mem_ptr.reset(open_segment(path)) ;
   }
    static std::string convert_base_dir([[maybe_unused]] const std::string &base_dir) {
        return "" ;
    }

    template<typename Function>
    static void attach( Function && f ) {
       f() ;
    }
};  

struct Mapped {
    typedef boost::interprocess::managed_mapped_file   segment_t;  
    typedef boost::interprocess::managed_mapped_file::segment_manager segment_manager_t;
    typedef boost::shared_mutex lock_t ;
    typedef boost::unique_lock<lock_t>  scoped_exclusive_locker_t;
    typedef boost::shared_lock<lock_t>  scoped_shared_locker_t;
    static segment_t * open_or_create_segment (const std::string &path, size_t size) {
        return new segment_t(boost::interprocess::open_or_create, path.c_str(), size) ; 
    }
    static segment_t * open_segment (const std::string &path) {
        return new segment_t(boost::interprocess::open_only, path.c_str()) ; 
    }
    static segment_t * create_segment (const std::string &path, size_t size) {
        return new segment_t(boost::interprocess::create_only, path.c_str(), size) ; 
    }
    template <typename MemPtr>
    static void grow( MemPtr &mem_ptr, const std::string &path, size_t size) {
        mem_ptr.reset() ;
        segment_t::grow(path.c_str(), size) ;
        mem_ptr.reset(open_segment(path)) ;
   }
    static std::string convert_base_dir(const std::string &base_dir) {
        return base_dir + "/";
    }

    template<typename Function>
    static void attach( Function && f ) {
       f() ;
    }
};

struct Heap {
    typedef boost::interprocess::managed_heap_memory   segment_t;
    typedef boost::interprocess::managed_heap_memory::segment_manager  segment_manager_t;
    typedef boost::shared_mutex lock_t ;
    typedef boost::unique_lock<lock_t>  scoped_exclusive_locker_t;
    typedef boost::shared_lock<lock_t>  scoped_shared_locker_t;
    static segment_t * open_or_create_segment ([[maybe_unused]] const std::string &path, size_t size) {
        return new segment_t(size) ; 
    }
    static segment_t * create_segment ([[maybe_unused]] const std::string &path, size_t size) {
        return new segment_t(size) ; 
    }
    template <typename MemPtr>
    static void grow( MemPtr &mem_ptr, [[maybe_unused]] const std::string &path, size_t size) {
        mem_ptr->grow(size) ;
   }
    static std::string convert_base_dir([[maybe_unused]] const std::string &base_dir) {
        return "" ;
    }

    template<typename Function>
    static void attach( Function && f) {
        f() ;
    }
};

}
#endif	/* __IPC_MEMORY_TYPES_HPP__ */
