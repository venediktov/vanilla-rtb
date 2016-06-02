/** \file jsonv/detail/basic_view.hpp
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_BASIC_VIEW_HPP_INCLUDED__
#define __JSONV_DETAIL_BASIC_VIEW_HPP_INCLUDED__

#include <jsonv/config.hpp>

#include <iterator>
#include <utility>

namespace jsonv
{
namespace detail
{

/** A view template used for array and object views of a \c value. This class allows traversing a \c value with a range
 *  based for loop.
 *  
 *  \note
 *  A view does nothing to preserve the lifetime of the underlying container, nor does it remain valid if the container
 *  is modified.
**/
template <typename TIterator,
          typename TConstIterator = TIterator
         >
class basic_view
{
public:
    typedef TIterator                                           iterator;
    typedef TConstIterator                                      const_iterator;
    typedef std::reverse_iterator<iterator>                     reverse_iterator;
    typedef std::reverse_iterator<const_iterator>               const_reverse_iterator;
    typedef typename std::iterator_traits<iterator>::value_type value_type;
    typedef typename std::iterator_traits<iterator>::reference  reference;
    typedef typename std::iterator_traits<iterator>::pointer    pointer;
    
public:
    basic_view(iterator begin_, iterator end_) :
            _begin(begin_),
            _end(end_)
    { }
    
    iterator       begin()       { return _begin; }
    const_iterator begin() const { return _begin; }
    iterator       end()         { return _end; }
    const_iterator end() const   { return _end; }
    
    const_iterator cbegin() const { return _begin; }
    const_iterator cend()   const { return _end; }
    
    reverse_iterator       rbegin()       { return reverse_iterator(end()); };
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    reverse_iterator       rend()         { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const   { return reverse_iterator(begin()); }
    
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const   { return reverse_iterator(begin()); }
    
private:
    iterator _begin;
    iterator _end;
};

/** Something that owns an object. Used in basic_owning_view to move the value somewhere fixed before getting the
 *  itertors and constructing the base.
**/
template <typename T>
class basic_owner
{
public:
    explicit basic_owner(T&& x) :
            _value(std::move(x))
    { }
    
protected:
    T _value;
};

/** A form of basic_view that owns the object it is iterating over. **/
template <typename TContainer,
          typename TIterator,
          typename TConstIterator = TIterator
         >
class basic_owning_view :
        private basic_owner<TContainer>,
        public basic_view<TIterator, TConstIterator>
{
    using basic_owner<TContainer>::_value;
    
public:
    template <typename FBegin, typename FEnd>
    basic_owning_view(TContainer&& container, FBegin begin, FEnd end) :
            basic_owner<TContainer>(std::move(container)),
            basic_view<TIterator, TConstIterator>(begin(_value), end(_value))
    { }
};

}
}

#endif/*__JSONV_DETAIL_BASIC_VIEW_HPP_INCLUDED__*/
