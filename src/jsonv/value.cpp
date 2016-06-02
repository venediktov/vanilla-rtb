/** \file
 *  
 *  Copyright (c) 2012-2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/value.hpp>
#include <jsonv/algorithm.hpp>
#include <jsonv/encode.hpp>
#include <jsonv/path.hpp>

#include "array.hpp"
#include "char_convert.hpp"
#include "detail.hpp"
#include "object.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>
#include <ostream>
#include <sstream>

namespace jsonv
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// kind                                                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const kind& k)
{
    switch (k)
    {
        case jsonv::kind::array:   return os << "array";
        case jsonv::kind::boolean: return os << "boolean";
        case jsonv::kind::decimal: return os << "decimal";
        case jsonv::kind::integer: return os << "integer";
        case jsonv::kind::null:    return os << "null";
        case jsonv::kind::object:  return os << "object";
        case jsonv::kind::string:  return os << "string";
        default:            return os << "kind(" << static_cast<int>(k) << ")";
    }
}

std::string to_string(const kind& k)
{
    std::ostringstream ss;
    ss << k;
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// kind_error                                                                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

kind_error::kind_error(const std::string& description) :
        std::logic_error(description)
{ }

kind_error::~kind_error() noexcept
{ }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// value                                                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

value::value(const std::string& val) :
        _kind(jsonv::kind::null)
{
    _data.string = new detail::string_impl;
    _kind = jsonv::kind::string;
    _data.string->_string = val;
}

value::value(const char* val) :
        value(std::string(val))
{ }

value::value(const std::wstring& val) :
        value(detail::convert_to_narrow(val))
{ }

value::value(const wchar_t* val) :
        value(detail::convert_to_narrow(val))
{ }

value::value(int64_t val) :
        _kind(jsonv::kind::integer)
{
    _data.integer = val;
}

value::value(double val) :
        _kind(jsonv::kind::decimal)
{
    _data.decimal = val;
}

value::value(float val) :
        value(double(val))
{ }

value::value(bool val) :
        _kind(jsonv::kind::boolean)
{
    _data.boolean = val;
}

#define JSONV_VALUE_INTEGER_ALTERNATIVE_CTOR_GENERATOR(type_)              \
    value::value(type_ val) :                                              \
            _kind(jsonv::kind::integer)                                           \
    {                                                                      \
        _data.integer = val;                                               \
    }
JSONV_INTEGER_ALTERNATES_LIST(JSONV_VALUE_INTEGER_ALTERNATIVE_CTOR_GENERATOR)

value::~value() noexcept
{
    clear();
}

value::value(const value& other) :
        _kind(other._kind)
{
    switch (other.kind())
    {
    case jsonv::kind::object:
        _data.object = other._data.object->clone();
        break;
    case jsonv::kind::array:
        _data.array = other._data.array->clone();
        break;
    case jsonv::kind::string:
        _data.string = other._data.string->clone();
        break;
    case jsonv::kind::integer:
        _data.integer = other._data.integer;
        break;
    case jsonv::kind::decimal:
        _data.decimal = other._data.decimal;
        break;
    case jsonv::kind::boolean:
        _data.boolean = other._data.boolean;
        break;
    case jsonv::kind::null:
        break;
    }
}

value& value::operator=(const value& other)
{
    value copy(other);
    swap(copy);
    return *this;
}

value::value(value&& other) noexcept :
        _data(other._data),
        _kind(other._kind)
{
    other._data.object = 0;
    other._kind = jsonv::kind::null;
}

value& value::operator=(value&& source) noexcept
{
    if (this != &source)
    {
        clear();
        
        _data = source._data;
        _kind = source._kind;
        source._data.object = 0;
        source._kind = jsonv::kind::null;
    }
    
    return *this;
}

bool value::is_array() const
{
    return kind() == jsonv::kind::array;
}

bool value::is_boolean() const
{
    return kind() == jsonv::kind::boolean;
}

bool value::is_decimal() const
{
    return kind() == jsonv::kind::decimal || kind() == jsonv::kind::integer;
}

bool value::is_integer() const
{
    return kind() == jsonv::kind::integer;
}

bool value::is_null() const
{
    return kind() == jsonv::kind::null;
}

bool value::is_string() const
{
    return kind() == jsonv::kind::string;
}

bool value::is_object() const
{
    return kind() == jsonv::kind::object;
}

template <typename TValueRef, typename TPathIterator, typename FOnNonexistantPath>
TValueRef walk_path(TValueRef&&               current,
                    TPathIterator             first,
                    TPathIterator             last,
                    const FOnNonexistantPath& on_nonexistant_path
                   )
{
    if (first == last)
        return current;
    
    const path_element& elem = *first;
    switch (elem.kind())
    {
    case path_element_kind::array_index:
        check_type({ jsonv::kind::array, jsonv::kind::null }, current.kind());
        if (current.kind() != jsonv::kind::array || elem.index() >= current.size())
            on_nonexistant_path(elem, current);
        return walk_path(current.at(elem.index()), std::next(first), last, on_nonexistant_path);
    case path_element_kind::object_key:
        check_type({ jsonv::kind::object, jsonv::kind::null }, current.kind());
        if (current.kind() != jsonv::kind::object || !current.count(elem.key()))
            on_nonexistant_path(elem, current);
        return walk_path(current.at(elem.key()), std::next(first), last, on_nonexistant_path);
    default:
        throw std::runtime_error(to_string(elem));
    }
}

value& value::at_path(const jsonv::path& p)
{
    return walk_path(*this,
                     p.begin(),
                     p.end(),
                     [&p] (const path_element& elem, const value& current)
                     {
                         throw std::out_of_range(to_string(elem) + " does not exist on " + to_string(current)
                                                 + " (full path: " + to_string(p) + ")"
                                                );
                     }
                    );
}

value& value::at_path(string_view path_description)
{
    return at_path(jsonv::path::create(path_description));
}

value& value::at_path(size_type path_idx)
{
    return at_path(jsonv::path({ path_element(path_idx) }));
}

const value& value::at_path(const jsonv::path& p) const
{
    return walk_path(*this,
                     p.begin(),
                     p.end(),
                     [&p] (const path_element& elem, const value& current)
                     {
                         throw std::out_of_range(to_string(elem) + " does not exist on " + to_string(current)
                                                 + " (full path: " + to_string(p) + ")"
                                                );
                     }
                    );
}

const value& value::at_path(string_view path_description) const
{
    return at_path(jsonv::path::create(path_description));
}

const value& value::at_path(size_type path_idx) const
{
    return at_path(jsonv::path({ path_element(path_idx) }));
}

value& value::path(const jsonv::path& p)
{
    return walk_path(*this,
                     p.begin(),
                     p.end(),
                     [] (const path_element& elem, value& current)
                     {
                         switch (elem.kind())
                         {
                         case path_element_kind::array_index:
                             if (current.kind() == jsonv::kind::null)
                                 current = array();
                             current.resize(elem.index() + 1);
                             break;
                         case path_element_kind::object_key:
                             if (current.kind() == jsonv::kind::null)
                                 current = object();
                             current.insert({ elem.key(), null });
                             break;
                         }
                     }
                    );
}

value& value::path(string_view path_description)
{
    return path(jsonv::path::create(path_description));
}

value& value::path(size_type path_idx)
{
    return path(jsonv::path({ path_element(path_idx) }));
}

value::size_type value::count_path(const jsonv::path& p) const
{
    // TODO: Performance of this function sucks!
    try
    {
        at_path(p);
        return 1;
    }
    catch (const std::out_of_range&)
    {
        return 0;
    }
    catch (const kind_error&)
    {
        return 0;
    }
}

value::size_type value::count_path(string_view p) const
{
    return count_path(jsonv::path::create(p));
}

value::size_type value::count_path(size_type p) const
{
    return count_path(jsonv::path({ p }));
}

void value::swap(value& other) noexcept
{
    using std::swap;
    
    // All types of this union a trivially swappable
    swap(_data, other._data);
    swap(_kind, other._kind);
}

void value::clear()
{
    switch (_kind)
    {
    case jsonv::kind::object:
        delete _data.object;
        break;
    case jsonv::kind::array:
        delete _data.array;
        break;
    case jsonv::kind::string:
        delete _data.string;
        break;
    case jsonv::kind::integer:
    case jsonv::kind::decimal:
    case jsonv::kind::boolean:
    case jsonv::kind::null:
        // do nothing
        break;
    }
    
    _kind = jsonv::kind::null;
    _data.object = 0;
}

const std::string& value::as_string() const
{
    check_type(jsonv::kind::string, _kind);
    return _data.string->_string;
}

std::wstring value::as_wstring() const
{
    return detail::convert_to_wide(as_string());
}

int64_t value::as_integer() const
{
    check_type(jsonv::kind::integer, _kind);
    return _data.integer;
}

double value::as_decimal() const
{
    if (_kind == jsonv::kind::integer)
        return double(as_integer());
    check_type(jsonv::kind::decimal, _kind);
    return _data.decimal;
}

bool value::as_boolean() const
{
    check_type(jsonv::kind::boolean, _kind);
    return _data.boolean;
}

bool value::operator==(const value& other) const
{
    if (this == &other && kind_valid(kind()))
        return true;
    else
        return compare(other) == 0;
}

bool value::operator !=(const value& other) const
{
    // must be first: an invalid type is not equal to itself
    if (!kind_valid(kind()))
        return true;
    
    if (this == &other)
        return false;
    else
        return compare(other) != 0;
}

int value::compare(const value& other) const
{
    using jsonv::compare;
    
    return compare(*this, other);
}

bool value::operator< (const value& other) const
{
    return compare(other) < 0;
}

bool value::operator<=(const value& other) const
{
    return compare(other) <= 0;
}

bool value::operator> (const value& other) const
{
    return compare(other) > 0;
}

bool value::operator>=(const value& other) const
{
    return compare(other) >= 0;
}

std::ostream& operator<<(std::ostream& stream, const value& val)
{
    ostream_encoder encoder(stream);
    encoder.encode(val);
    return stream;
}

std::string to_string(const value& val)
{
    std::ostringstream os;
    os << val;
    return os.str();
}

bool value::empty() const
{
    check_type({ jsonv::kind::object, jsonv::kind::array, jsonv::kind::string }, kind());
    
    switch (kind())
    {
    case jsonv::kind::object:
        return _data.object->empty();
    case jsonv::kind::array:
        return _data.array->empty();
    case jsonv::kind::string:
        return _data.string->_string.empty();
    case jsonv::kind::integer:
    case jsonv::kind::decimal:
    case jsonv::kind::boolean:
    case jsonv::kind::null:
    default:
        // Should never hit this...
        return false;
    }
}

value::size_type value::size() const
{
    check_type({ jsonv::kind::object, jsonv::kind::array, jsonv::kind::string }, kind());
    
    switch (kind())
    {
    case jsonv::kind::object:
        return _data.object->size();
    case jsonv::kind::array:
        return _data.array->size();
    case jsonv::kind::string:
        return _data.string->_string.size();
    case jsonv::kind::integer:
    case jsonv::kind::decimal:
    case jsonv::kind::boolean:
    case jsonv::kind::null:
    default:
        // Should never hit this...
        return false;
    }
}

value value::map(const std::function<value (const value&)>& func) const&
{
    return jsonv::map(func, *this);
}

value value::map(const std::function<value (value)>& func) &&
{
    return jsonv::map(func, std::move(*this));
}

void swap(value& a, value& b) noexcept
{
    a.swap(b);
}

// There are no static initialization issues here -- the memory of a static variable starts as all 0, which is identical
// to a value with kind::null.
const value null = value();

}

namespace std
{

template <typename TForwardIterator, typename FHasher>
static std::size_t hash_range(TForwardIterator first, TForwardIterator last, const FHasher& hasher)
{
    std::size_t x = 0;
    for ( ; first != last; ++first)
        x = (x << 1) ^ hasher(*first);
    
    return x;
}

size_t hash<jsonv::value>::operator()(const jsonv::value& val) const noexcept
{
    using namespace jsonv;
    
    switch (val.kind())
    {
    case jsonv::kind::object:
        return hash_range(val.begin_object(), val.end_object(),
                          [] (const value::object_value_type& x) { return std::hash<std::string>()(x.first)
                                                                        ^ std::hash<value>()(x.second);
                                                                 }
                         );
    case jsonv::kind::array:
        return hash_range(val.begin_array(), val.end_array(), hash<jsonv::value>());
    case jsonv::kind::string:
        return std::hash<std::string>()(val.as_string());
    case jsonv::kind::integer:
        return std::hash<std::int64_t>()(val.as_integer());
    case jsonv::kind::decimal:
        return std::hash<double>()(val.as_decimal());
    case jsonv::kind::boolean:
        return std::hash<bool>()(val.as_boolean());
    case jsonv::kind::null:
        return 0x51afb2fe9467d0f7ULL;
    default:
        // Should never hit this...
        return 0ULL;
    }
}

}
