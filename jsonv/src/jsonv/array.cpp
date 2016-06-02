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
#include <jsonv/array.hpp>

#include "detail.hpp"

#include <algorithm>
#include <ostream>

namespace jsonv
{

value array()
{
    value x;
    x._data.array = new detail::array_impl;
    x._kind = jsonv::kind::array;
    return x;
}

value array(std::initializer_list<value> source)
{
    value x = array();
    x.assign(source);
    return x;
}

namespace detail
{

value::size_type array_impl::size() const
{
    return _values.size();
}

bool array_impl::empty() const
{
    return _values.empty();
}

}

value::array_iterator value::begin_array()
{
    check_type(jsonv::kind::array, kind());
    return array_iterator(this, 0);
}

value::const_array_iterator value::begin_array() const
{
    check_type(jsonv::kind::array, kind());
    return const_array_iterator(this, 0);
}

value::array_iterator value::end_array()
{
    check_type(jsonv::kind::array, kind());
    return array_iterator(this, _data.array->_values.size());
}

value::const_array_iterator value::end_array() const
{
    check_type(jsonv::kind::array, kind());
    return const_array_iterator(this, _data.array->_values.size());
}

value::array_view value::as_array() &
{
    return array_view(begin_array(), end_array());
}

value::const_array_view value::as_array() const &
{
    return const_array_view(begin_array(), end_array());
}

value::owning_array_view value::as_array() &&
{
    check_type(jsonv::kind::array, kind());
    return owning_array_view(std::move(*this),
                             [] (value& x) { return x.begin_array(); },
                             [] (value& x) { return x.end_array(); }
                            );
}

value& value::operator[](size_type idx)
{
    check_type(jsonv::kind::array, kind());
    return _data.array->_values[idx];
}

const value& value::operator[](size_type idx) const
{
    check_type(jsonv::kind::array, kind());
    return _data.array->_values[idx];
}

value& value::at(size_type idx)
{
    check_type(jsonv::kind::array, kind());
    return _data.array->_values.at(idx);
}

const value& value::at(size_type idx) const
{
    check_type(jsonv::kind::array, kind());
    return _data.array->_values.at(idx);
}

void value::push_back(value item)
{
    check_type(jsonv::kind::array, kind());
    _data.array->_values.emplace_back(std::move(item));
}

void value::pop_back()
{
    check_type(jsonv::kind::array, kind());
    if (_data.array->_values.empty())
        throw std::logic_error("Cannot pop from empty array");
    _data.array->_values.pop_back();
}

void value::push_front(value item)
{
    check_type(jsonv::kind::array, kind());
    _data.array->_values.emplace_front(std::move(item));
}

void value::pop_front()
{
    check_type(jsonv::kind::array, kind());
    if (_data.array->_values.empty())
        throw std::logic_error("Cannot pop from empty array");
    _data.array->_values.pop_front();
}

value::array_iterator value::insert(const_array_iterator position, value item)
{
    check_type(jsonv::kind::array, kind());
    auto iter = _data.array->_values.begin() + std::distance(const_array_iterator(begin_array()), position);
    iter = _data.array->_values.insert(iter, std::move(item));
    return begin_array() + std::distance(_data.array->_values.begin(), iter);
}

void value::assign(size_type count, const value& val)
{
    check_type(jsonv::kind::array, kind());
    _data.array->_values.assign(count, val);
}

void value::assign(std::initializer_list<value> items)
{
    check_type(jsonv::kind::array, kind());
    _data.array->_values.assign(std::move(items));
}

void value::resize(size_type count, const value& val)
{
    check_type(jsonv::kind::array, kind());
    _data.array->_values.resize(count, val);
}

value::array_iterator value::erase(const_array_iterator position)
{
    check_type(jsonv::kind::array, kind());
    difference_type dist(position - begin_array());
    _data.array->_values.erase(_data.array->_values.begin() + dist);
    return array_iterator(this, static_cast<size_type>(dist));
}

value::array_iterator value::erase(const_array_iterator first, const_array_iterator last)
{
    difference_type fdist(first - begin_array());
    difference_type ldist(last  - begin_array());
    _data.array->_values.erase(_data.array->_values.begin() + fdist,
                               _data.array->_values.begin() + ldist
                              );
    return array_iterator(this, static_cast<size_type>(fdist));
}

}
