/*
 * File:   base_entity.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on February 1, 2017, 11:57 PM
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
#pragma once 
#ifndef __IPC_DATA_BASE_ENTITY_HPP__
#define __IPC_DATA_BASE_ENTITY_HPP__
 
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include <sstream>

namespace ipc { namespace data {

    template <typename Alloc>
    struct base_entity
    {
    protected:
        using char_string =  boost::interprocess::basic_string<char, std::char_traits<char>, Alloc> ;
        base_entity( const Alloc & a ) : allocator{a}, blob{a}
        {} 

        Alloc allocator ;
        char_string blob;

        using ostreambuf_t = boost::iostreams::stream_buffer<boost::iostreams::back_insert_device<char_string>>;
        using istreambuf_t = boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>>;

        using oarchive_t = boost::archive::binary_oarchive;
        using iarchive_t = boost::archive::binary_iarchive;

        template<typename Serializable>
        void store(Serializable  && data)  {
            ostreambuf_t out{blob};
            boost::archive::binary_oarchive oarch(out);
            oarch << std::forward<Serializable>(data) ;
        }

        template<typename Serializable>
        static std::size_t size(const Serializable && data) {
            std::stringstream ss;
            oarchive_t oarch(ss);
            oarch << std::forward<Serializable>(data) ;
            return sizeof(data.allocator) + ss.str().size() ;
        }

        template<typename Serializable>
        void retrieve(Serializable  & data) const {
            istreambuf_t inp{blob.data(), blob.length()};
            iarchive_t iarch(inp);
            iarch >> data;
        }

        void operator()(base_entity &entry) const {
            entry.blob=blob;
        }
    };
}}
 
#endif     /* __IPC_DATA_BASE_ENTITY_HPP__  */


