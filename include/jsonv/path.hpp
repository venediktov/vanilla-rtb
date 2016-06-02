/** \file jsonv/path.hpp
 *  Support for [JSONPath](http://goessner.net/articles/JsonPath/).
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_PATH_HPP_INCLUDED__
#define __JSONV_PATH_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/detail/generic_container.hpp>
#include <jsonv/string_view.hpp>

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace jsonv
{

enum class path_element_kind : unsigned char
{
    array_index,
    object_key,
};

JSONV_PUBLIC std::ostream& operator<<(std::ostream&, const path_element_kind&);

JSONV_PUBLIC std::string to_string(const path_element_kind&);

class JSONV_PUBLIC path_element
{
public:
    path_element(std::size_t idx);
    path_element(int         idx);
    path_element(std::string key);
    path_element(string_view  key);
    path_element(const char* key);
    path_element(const path_element&);
    path_element& operator=(const path_element&);
    path_element(path_element&&) noexcept;
    path_element& operator=(path_element&&) noexcept;
    
    ~path_element() noexcept;
    
    path_element_kind kind() const;
    
    std::size_t index() const;
    
    const std::string& key() const;
    
    bool operator==(const path_element&) const;
    bool operator!=(const path_element&) const;
    
private:
    union JSONV_PUBLIC storage
    {
        std::size_t index;
        std::string key;
        
        storage(std::size_t   idx);
        storage(std::string&& key);
        ~storage() noexcept;
    };
    
private:
    path_element_kind _kind;
    storage           _data;
};

JSONV_PUBLIC std::ostream& operator<<(std::ostream&, const path_element&);

JSONV_PUBLIC std::string to_string(const path_element&);

/** Represents an exact path in some JSON structure. **/
class JSONV_PUBLIC path :
        public detail::generic_container<std::vector<path_element>>
{
public:
    /** Creates a new, empty path. **/
    path();
    
    /** Creates a path with the provided \a elements. **/
    path(storage_type elements);
    
    /** Create a \c path from a string definition. The syntax of this is ECMAScript's syntax for selecting elements, so
     *  <tt>path::create(".foo.bar[1]")</tt> is equivalent to <tt>path({ "foo", "bar", 1 })</tt>.
     *  
     *  \throws std::invalid_argument if the \a specification is not valid.
    **/
    static path create(string_view specification);
    
    path(const path&);
    path& operator=(const path&);
    path(path&&) noexcept;
    path& operator=(path&&) noexcept;
    ~path() noexcept;
    
    /** Return a new path with the given \a subpath appended to the back. **/
    path  operator+(const path& subpath) const;
    path& operator+=(const path& subpath);
    
    /** Return a new path with the given \a elem appended to the back. **/
    path  operator+(path_element elem) const;
    path& operator+=(path_element elem);
};

JSONV_PUBLIC std::ostream& operator<<(std::ostream&, const path&);

JSONV_PUBLIC std::string to_string(const path&);

}

#endif/*__JSONV_PATH_HPP_INCLUDED__*/
