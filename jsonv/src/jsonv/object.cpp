/** \file
 *  Implementation of \c jsonv::value member functions related to objects.
 *  
 *  Copyright (c) 2012-2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/object.hpp>
#include <jsonv/char_convert.hpp>

#include <algorithm>
#include <cstring>

namespace jsonv
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// object                                                                                                             //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

value object()
{
    value x;
    x._data.object = new detail::object_impl;
    x._kind = jsonv::kind::object;
    return x;
}

value object(std::initializer_list<std::pair<std::string, value>> source)
{
    value x = object();
    x.insert(std::move(source));
    return x;
}

value object(std::initializer_list<std::pair<std::wstring, value>> source)
{
    value x = object();
    x.insert(std::move(source));
    return x;
}

value::object_iterator value::begin_object()
{
    check_type(jsonv::kind::object, kind());
    return object_iterator(_data.object->_values.begin());
}

value::const_object_iterator value::begin_object() const
{
    check_type(jsonv::kind::object, kind());
    return const_object_iterator(_data.object->_values.begin());
}

value::object_iterator value::end_object()
{
    check_type(jsonv::kind::object, kind());
    return object_iterator(_data.object->_values.end());
}

value::const_object_iterator value::end_object() const
{
    check_type(jsonv::kind::object, kind());
    return const_object_iterator(_data.object->_values.end());
}

value::object_view value::as_object() &
{
    return object_view(begin_object(), end_object());
}

value::const_object_view value::as_object() const &
{
    return const_object_view(begin_object(), end_object());
}

value::owning_object_view value::as_object() &&
{
    check_type(jsonv::kind::object, kind());
    return owning_object_view(std::move(*this),
                              [] (value& x) { return x.begin_object(); },
                              [] (value& x) { return x.end_object(); }
                             );
}

value& value::operator[](const std::string& key)
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values[key];
}

value& value::operator[](std::string&& key)
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values[std::move(key)];
}

value& value::operator[](const std::wstring& key)
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values[detail::convert_to_narrow(key)];
}

value& value::at(const std::string& key)
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.at(key);
}

const value& value::at(const std::string& key) const
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.at(key);
}

value& value::at(const std::wstring& key)
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.at(detail::convert_to_narrow(key));
}

const value& value::at(const std::wstring& key) const
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.at(detail::convert_to_narrow(key));
}

value::size_type value::count(const std::string& key) const
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.count(key);
}

value::size_type value::count(const std::wstring& key) const
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.count(detail::convert_to_narrow(key));
}

value::object_iterator value::find(const std::string& key)
{
    check_type(jsonv::kind::object, kind());
    return object_iterator(_data.object->_values.find(key));
}

value::object_iterator value::find(const std::wstring& key)
{
    check_type(jsonv::kind::object, kind());
    return object_iterator(_data.object->_values.find(detail::convert_to_narrow(key)));
}

value::const_object_iterator value::find(const std::string& key) const
{
    check_type(jsonv::kind::object, kind());
    return const_object_iterator(_data.object->_values.find(key));
}

value::const_object_iterator value::find(const std::wstring& key) const
{
    check_type(jsonv::kind::object, kind());
    return const_object_iterator(_data.object->_values.find(detail::convert_to_narrow(key)));
}

value::object_iterator value::insert(value::const_object_iterator hint, std::pair<std::string, value> pair)
{
    check_type(jsonv::kind::object, kind());
    return object_iterator(_data.object->_values.insert(hint._impl, std::move(pair)));
}

value::object_iterator value::insert(value::const_object_iterator hint, std::pair<std::wstring, value> pair)
{
    check_type(jsonv::kind::object, kind());
    return insert(hint, { detail::convert_to_narrow(pair.first), std::move(pair.second) });
}

std::pair<value::object_iterator, bool> value::insert(std::pair<std::string, value> pair)
{
    check_type(jsonv::kind::object, kind());
    auto ret = _data.object->_values.insert(pair);
    return { object_iterator(ret.first), ret.second };
}

std::pair<value::object_iterator, bool> value::insert(std::pair<std::wstring, value> pair)
{
    check_type(jsonv::kind::object, kind());
    auto ret = _data.object->_values.insert({ detail::convert_to_narrow(pair.first), std::move(pair.second) });
    return { object_iterator(ret.first), ret.second };
}

void value::insert(std::initializer_list<std::pair<std::string, value>> items)
{
    check_type(jsonv::kind::object, kind());
    for (auto& pair : items)
         _data.object->_values.insert(std::move(pair));
}

void value::insert(std::initializer_list<std::pair<std::wstring, value>> items)
{
    check_type(jsonv::kind::object, kind());
    for (auto& pair : items)
         insert(std::move(pair));
}

value::size_type value::erase(const std::string& key)
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.erase(key);
}

value::size_type value::erase(const std::wstring& key)
{
    check_type(jsonv::kind::object, kind());
    return _data.object->_values.erase(detail::convert_to_narrow(key));
}

value::object_iterator value::erase(const_object_iterator position)
{
    check_type(jsonv::kind::object, kind());
    return object_iterator(_data.object->_values.erase(position._impl));
}

value::object_iterator value::erase(const_object_iterator first, const_object_iterator last)
{
    check_type(jsonv::kind::object, kind());
    return object_iterator(_data.object->_values.erase(first._impl, last._impl));
}

namespace detail
{

bool object_impl::empty() const
{
    return _values.empty();
}

value::size_type object_impl::size() const
{
    return _values.size();
}

}
}
