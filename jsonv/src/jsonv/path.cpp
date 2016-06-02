/** \file
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/path.hpp>
#include <jsonv/value.hpp>
#include <jsonv/detail.hpp>
#include <jsonv/char_convert.hpp>
#include <jsonv/detail/token_patterns.hpp>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <ostream>
#include <sstream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

namespace jsonv
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// path_element                                                                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const path_element_kind& val)
{
    switch (val)
    {
    case path_element_kind::array_index: return os << "array_index";
    case path_element_kind::object_key:  return os << "object_key";
    default:                             return os << "path_element_kind(" << static_cast<int>(val) << ")";
    }
}

std::string to_string(const path_element_kind& val)
{
    std::ostringstream ss;
    ss << val;
    return ss.str();
}

path_element::storage::storage(std::size_t idx) :
        index(idx)
{ }

path_element::storage::storage(std::string&& key) :
        key(std::move(key))
{ }

path_element::storage::~storage() noexcept
{
    // do nothing -- must be handled in path_element::~path_element
}

path_element::path_element(std::size_t idx) :
        _kind(path_element_kind::array_index),
        _data(idx)
{ }

path_element::path_element(int idx) :
        path_element(std::size_t(idx))
{ }

path_element::path_element(std::string key) :
        _kind(path_element_kind::object_key),
        _data(std::move(key))
{ }

path_element::path_element(string_view key) :
        path_element(std::string(key))
{ }

path_element::path_element(const char* key) :
        path_element(std::string(key))
{ }

path_element::path_element(const path_element& src) :
        _kind(src._kind),
        _data(0)
{
    if (_kind == path_element_kind::array_index)
        _data.index = src._data.index;
    else
        new(static_cast<void*>(&_data.key)) std::string(src._data.key);
}

path_element& path_element::operator=(const path_element& src)
{
    if (src._kind == path_element_kind::array_index)
    {
        if (_kind == path_element_kind::array_index)
        {
            _data.index = src._data.index;
        }
        else
        {
            _data.key.~basic_string();
            _kind = path_element_kind::array_index;
            _data.index = src._data.index;
        }
    }
    else // src._kind == path_element_kind::object_key
    {
        if (_kind == path_element_kind::array_index)
        {
            // place a copied string into our data
            new(static_cast<void*>(&_data.key)) std::string(src._data.key);
            _kind = path_element_kind::object_key;
        }
        else
        {
            _data.key = src._data.key;
        }
    }
    
    return *this;
}

path_element::path_element(path_element&& src) noexcept :
        _kind(src._kind),
        _data(0)
{
    if (_kind == path_element_kind::array_index)
        _data.index = src._data.index;
    else
        new(static_cast<void*>(&_data.key)) std::string(std::move(src._data.key));
}

path_element& path_element::operator=(path_element&& src) noexcept
{
    if (src._kind == path_element_kind::array_index)
    {
        if (_kind == path_element_kind::array_index)
        {
            _data.index = src._data.index;
        }
        else
        {
            _data.key.~basic_string();
            _kind = path_element_kind::array_index;
            _data.index = src._data.index;
        }
    }
    else // src._kind == path_element_kind::object_key
    {
        if (_kind == path_element_kind::array_index)
        {
            // place a moved string into our data
            new(static_cast<void*>(&_data.key)) std::string(std::move(src._data.key));
            _kind = path_element_kind::object_key;
        }
        else
        {
            _data.key = std::move(src._data.key);
        }
    }
    
    return *this;
}

path_element::~path_element() noexcept
{
    if (_kind == path_element_kind::object_key)
    {
        _data.key.~basic_string();
        _kind = path_element_kind::array_index;
    }
}

path_element_kind path_element::kind() const
{
    return _kind;
}

std::size_t path_element::index() const
{
    if (_kind != path_element_kind::array_index)
        throw kind_error("Cannot get index on object_key path_element");
    else
        return _data.index;
}

const std::string& path_element::key() const
{
    if (_kind != path_element_kind::object_key)
        throw kind_error("Cannot get key on array_index path_element");
    else
        return _data.key;
}

bool path_element::operator==(const path_element& other) const
{
    if (kind() != other.kind())
        return false;
    else if (kind() == path_element_kind::object_key)
        return key() == other.key();
    else
        return index() == other.index();
}

bool path_element::operator!=(const path_element& other) const
{
    return !operator==(other);
}

static std::ostream& stream_path_element(std::ostream& os, const path_element& elem)
{
    switch (elem.kind())
    {
    case path_element_kind::array_index:
        return os << '[' << elem.index() << ']';
    case path_element_kind::object_key:
        // if any of the elements is not alphanumeric (or it is an empty string), use the [] notation
        if (elem.key().empty() || std::any_of(begin(elem.key()), end(elem.key()), [] (char c) { return !std::isalnum(c); }))
            return os << "[\"" << elem.key() << "\"]";
        else
            return os << '.' << elem.key();
    default:
        return os << "path_element(invalid:" << elem.kind() << ")";
    }
}

std::ostream& operator<<(std::ostream& os, const path_element& elem)
{
    return stream_path_element(os, elem);
}

std::string to_string(const path_element& val)
{
    std::ostringstream os;
    os << val;
    return os.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// path                                                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

path::path()
{ }

path::path(std::vector<path_element> elements) :
        generic_container<std::vector<path_element>>(std::move(elements))
{ }

path::path(const path&) = default;
path& path::operator=(const path&) = default;

#ifdef _MSC_VER
path::path(path&& src) noexcept :
        detail::generic_container<std::vector<path_element>>(std::move(src))
{ }
#else
path::path(path&&) noexcept = default;
#endif

path& path::operator=(path&& src) noexcept
{
    _data = std::move(src._data);
    return *this;
}
path::~path() noexcept = default;

path path::create(string_view specification)
{
    path out;
    string_view remaining = specification;
    while (!remaining.empty())
    {
        string_view match;
        switch (detail::path_match(remaining, match))
        {
        case detail::path_match_result::simple_object:
                out += match.substr(1);
                break;
        case detail::path_match_result::brace:
            if (match.at(1) == '\"')
                out += detail::get_string_decoder(parse_options::encoding::utf8)(match.substr(2, match.size() - 4));
            else
                out += boost::lexical_cast<std::size_t>(match.data() + 1, match.size() - 2);
            break;
        default:
            throw std::invalid_argument(std::string("Invalid specification \"") + std::string(specification) + "\". "
                                        +"Syntax error at \"" + std::string(remaining) + "\""
                                       );
        }
            
        remaining.remove_prefix(match.size());
    }
    
    return out;
}

path& path::operator+=(const path& subpath)
{
    using std::begin; using std::end;
    _data.insert(end(_data), begin(subpath), end(subpath));
    return *this;
}

path path::operator+(const path& subpath) const
{
    path clone(*this);
    clone += subpath;
    return clone;
}

path& path::operator+=(path_element elem)
{
    _data.emplace_back(std::move(elem));
    return *this;
}

path path::operator+(path_element elem) const
{
    path clone(*this);
    clone += elem;
    return clone;
}

std::ostream& operator<<(std::ostream& os, const path& val)
{
    for (const path_element& elem : val)
    {
        stream_path_element(os, elem);
    }
    return os;
}

std::string to_string(const path& val)
{
    std::ostringstream os;
    os << val;
    return os.str();
}

}
