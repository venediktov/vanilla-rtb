/** \file
 *  Internal-only header for array-related values.
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_ARRAY_HPP_INCLUDED__
#define __JSONV_ARRAY_HPP_INCLUDED__

#include <jsonv/value.hpp>
#include <jsonv/detail.hpp>

#include <deque>

namespace jsonv
{
namespace detail
{

class JSONV_LOCAL array_impl :
        public cloneable<array_impl>
{
public:
    typedef std::deque<jsonv::value> array_type;
    
public:
    value::size_type size() const;
    
    bool empty() const;
    
public:
    array_type _values;
};

}
}


#endif/*__JSONV_ARRAY_HPP_INCLUDED__*/
