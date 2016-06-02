/** \file
 *  
 *  Copyright (c) 2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/algorithm.hpp>
#include <jsonv/value.hpp>

namespace jsonv
{

value map(const std::function<value (const value&)>& func,
          const value&                               input
         )
{
    switch (input.kind())
    {
    case kind::boolean:
    case kind::decimal:
    case kind::integer:
    case kind::null:
    case kind::string:
        return func(input);
    case kind::array:
    {
        value out = array();
        for (const value& sub : input.as_array())
            out.push_back(func(sub));
        return out;
    }
    case kind::object:
    {
        value out = object();
        for (const value::object_value_type& sub : input.as_object())
            out.insert({ sub.first, func(sub.second) });
        return out;
    }
    default:
        return null;
    }
}

value map(const std::function<value (value)>& func,
          value&&                             input
         )
{
    switch (input.kind())
    {
    case kind::boolean:
    case kind::decimal:
    case kind::integer:
    case kind::null:
    case kind::string:
        return func(std::move(input));
    case kind::array:
    {
        value out = array();
        for (value& sub : input.as_array())
            out.push_back(func(std::move(sub)));
        input = null;
        return out;
    }
    case kind::object:
    {
        value out = object();
        for (value::object_value_type& sub : input.as_object())
            out.insert({ sub.first, func(std::move(sub.second)) });
        input = null;
        return out;
    }
    default:
        return null;
    }
}

}
