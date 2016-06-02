/** \file jsonv/detail/fixed_map.hpp
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_FIXED_MAP_HPP_INCLUDED__
#define __JSONV_DETAIL_FIXED_MAP_HPP_INCLUDED__

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <functional>
#include <utility>

namespace jsonv
{
namespace detail
{

template <typename TKey, typename TValue, std::size_t KCount, typename FCompare = std::less<TKey>>
class fixed_map
{
public:
    typedef TKey                                   key_type;
    typedef TValue                                 mapped_type;
    typedef std::pair<const key_type, mapped_type> value_type;
    typedef FCompare                               key_compare;
    typedef value_type&                            reference;
    typedef const value_type&                      const_reference;
    typedef value_type*                            pointer;
    typedef const value_type*                      const_pointer;
    typedef pointer                                iterator;
    typedef const_pointer                          const_iterator;
    
public:
    fixed_map(const std::initializer_list<value_type>& init,
              key_compare cmp                               = key_compare()
             ) :
            _cmp(std::move(cmp))
    {
        using std::begin;
        using std::end;
        
        auto last = std::copy(begin(init), end(init), begin(_data));
        static_cast<void>(last);
        assert(last == end(_data));
        
        std::sort(begin(_data), end(_data), [this] (const value_type& a, const value_type& b) { return _cmp(a.first, b.first); });
    }
    
    iterator begin()             { return reinterpret_cast<iterator>(_data.data()); }
    const_iterator begin() const { return reinterpret_cast<const_iterator>(_data.data()); }
    iterator end()               { return begin() + KCount; }
    const_iterator end() const   { return begin() + KCount; }
    
    iterator find(const key_type& key)
    {
        auto iter = std::lower_bound(begin(), end(),
                                     key,
                                     [this] (const value_type& x, const key_type& k) { return _cmp(x.first, k); }
                                    );
        if (!_cmp(key, iter->first))
            return iter;
        else
            return this->end();
    }
    
    const_iterator find(const key_type& key) const
    {
        return const_cast<fixed_map*>(this)->find(key);
    }
    
private:
    std::array<std::pair<key_type, mapped_type>, KCount> _data;
    key_compare                                          _cmp;
};

}
}

#endif/*__JSONV_DETAIL_FIXED_MAP_HPP_INCLUDED__*/
