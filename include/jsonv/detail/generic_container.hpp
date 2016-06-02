/** \file jsonv/detail/generic_container.hpp
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_GENERIC_CONTAINER_HPP_INCLUDED__
#define __JSONV_DETAIL_GENERIC_CONTAINER_HPP_INCLUDED__

#include <jsonv/config.hpp>

#include <algorithm>
#include <initializer_list>
#include <utility>

namespace jsonv
{
namespace detail
{

/** A generic container that exposes the traversal and modification operations externally. Basically, this is just a way
 *  to allow deriving from \c std::vector without actually doing so.
**/
template <typename TStorage>
class generic_container
{
public:
    using storage_type           = TStorage;
    using size_type              = typename storage_type::size_type;
    using value_type             = typename storage_type::value_type;
    using difference_type        = typename storage_type::difference_type;
    using iterator               = typename storage_type::iterator;
    using const_iterator         = typename storage_type::const_iterator;
    using reverse_iterator       = typename storage_type::reverse_iterator;
    using const_reverse_iterator = typename storage_type::const_reverse_iterator;
    using reference              = typename storage_type::reference;
    using const_reference        = typename storage_type::const_reference;
    using pointer                = typename storage_type::pointer;
    using const_pointer          = typename storage_type::const_pointer;
    using allocator_type         = typename storage_type::allocator_type;
    
public:
    generic_container()
    { }
    
    explicit generic_container(storage_type data) :
            _data(std::move(data))
    { }
    
    template <typename TInputIterator>
    generic_container(TInputIterator first, TInputIterator last) :
            _data(first, last)
    { }
    
    generic_container(const generic_container&) = default;
    generic_container& operator=(const generic_container&) = default;
    generic_container(generic_container&&) noexcept = default;
    generic_container& operator=(generic_container&&) = default;
    
    /** Get the number of elements. **/
    size_type size() const { return _data.size(); }
    
    bool empty() const { return _data.empty(); }
    
    iterator       begin()        { return _data.begin(); }
    const_iterator begin() const  { return _data.begin(); }
    const_iterator cbegin() const { return _data.begin(); }
    
    iterator       end()        { return _data.end(); }
    const_iterator end() const  { return _data.end(); }
    const_iterator cend() const { return _data.end(); }
    
    reverse_iterator       rbegin()        { return _data.rbegin(); }
    const_reverse_iterator rbegin() const  { return _data.rbegin(); }
    const_reverse_iterator crbegin() const { return _data.rbegin(); }
    
    reverse_iterator       rend()        { return _data.rend(); }
    const_reverse_iterator rend() const  { return _data.rend(); }
    const_reverse_iterator crend() const { return _data.rend(); }
    
    reference       operator[](size_type idx)       { return _data[idx]; }
    const_reference operator[](size_type idx) const { return _data[idx]; }
    
    reference       at(size_type idx)       { return _data.at(idx); }
    const_reference at(size_type idx) const { return _data.at(idx); }
    
    reference       front()       { return _data.front(); }
    const_reference front() const { return _data.front(); }
    
    reference       back()       { return _data.back(); }
    const_reference back() const { return _data.back(); }
    
    void clear() { return _data.clear(); }
    
    iterator insert(const_iterator pos, const value_type& x) { return _data.insert(pos, x); }
    iterator insert(const_iterator pos, value_type&& x)      { return _data.insert(pos, std::move(x)); }
    
    template <typename TInputIterator>
    iterator insert(const_iterator pos, TInputIterator first, TInputIterator last)
    {
        return _data.insert(pos, first, last);
    }
    
    iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
    {
        return _data.insert(pos, ilist);
    }
    
    template <typename... TArgs>
    iterator emplace(const_iterator pos, TArgs&&... args)
    {
        return _data.emplace(pos, std::forward<TArgs>(args)...);
    }
    
    template <typename... TArgs>
    void emplace_back(TArgs&&... args)
    {
        return _data.emplace_back(std::forward<TArgs>(args)...);
    }
    
    void push_back(const value_type& x) { return _data.push_back(x); }
    void push_back(value_type&& x)      { return _data.push_back(std::move(x)); }
    
    iterator erase(const_iterator pos) { return _data.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return _data.erase(first, last); }
    
    void pop_back() { return _data.pop_back(); }
    
protected:
    storage_type _data;
};

template <typename TStorage>
bool operator==(const generic_container<TStorage>& a, const generic_container<TStorage>& b)
{
    if (&a == &b)
        return true;
    else if (a.size() != b.size())
        return false;
    else
        return std::equal(a.begin(), a.end(), b.begin());
}

template <typename TStorage>
bool operator!=(const generic_container<TStorage>& a, const generic_container<TStorage>& b)
{
    return !(a == b);
}

}
}

#endif/*__JSONV_DETAIL_GENERIC_CONTAINER_HPP_INCLUDED__*/
