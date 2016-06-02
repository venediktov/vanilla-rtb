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
#ifndef __JSONV_DETAIL_HPP_INCLUDED__
#define __JSONV_DETAIL_HPP_INCLUDED__

#include <jsonv/value.hpp>
#include <jsonv/string_view.hpp>

namespace jsonv
{
namespace detail
{

template <typename T>
struct cloneable
{
    T* clone() const
    {
        return new T(*static_cast<const T*>(this));
    }
};

class string_impl :
        public cloneable<string_impl>
{
public:
    std::string _string;
};

}

const char* kind_desc(kind type);
bool kind_valid(kind k);
void check_type(kind expected, kind actual);
void check_type(std::initializer_list<kind> expected, kind actual);
std::ostream& stream_escaped_string(std::ostream& stream, string_view str, bool require_ascii);

}

#endif/*__JSONV_DETAIL_HPP_INCLUDED__*/
