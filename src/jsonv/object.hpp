/** \file
 *  Internal-only header for object-related values.
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_OBJECT_HPP_INCLUDED__
#define __JSONV_OBJECT_HPP_INCLUDED__

#include <jsonv/value.hpp>
#include <jsonv/detail.hpp>

#include <map>

namespace jsonv
{
namespace detail
{

class JSONV_LOCAL object_impl :
        public cloneable<object_impl>
{
public:
    using map_type       = std::map<std::string, jsonv::value>;
    using iterator       = map_type::iterator;
    using const_iterator = map_type::const_iterator;
    
public:
    bool empty() const;
    
    value::size_type size() const;
        
public:
    map_type _values;
};

}

}

#endif/*__JSONV_OBJECT_HPP_INCLUDED__*/
