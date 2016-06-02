/** \file jsonv/functional.hpp
 *  A collection of function objects a la \c <functional>.
 *  
 *  Copyright (c) 2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_FUNCTIONAL_HPP_INCLUDED__
#define __JSONV_FUNCTIONAL_HPP_INCLUDED__

#include <jsonv/config.hpp>

#include <functional>

namespace jsonv
{

class value;

/** \addtogroup Algorithm
 *  \{
**/

/** Compares a \c value to another using the standard-issue \c value::compare function. **/
struct value_compare
{
    using first_argument_type  = value;
    using second_argument_type = value;
    using result_type          = int;
    
    int operator()(const value& a, const value& b) const;
};

/** Compares two values, ignoring the case for any \c value with \c kind::string. This does \e not ignore case for the
 *  keys of objects.
**/
struct value_compare_icase
{
    using first_argument_type  = value;
    using second_argument_type = value;
    using result_type          = int;
    
    int operator()(const value& a, const value& b) const;
};

template <typename FCompare, typename FResult>
struct basic_value_binary_predicate :
        private FCompare,
        private FResult
{
    using first_argument_type  = value;
    using second_argument_type = value;
    using result_type          = bool;
    
    bool operator()(const value& a, const value& b) const
    {
        return FResult::operator()(FCompare::operator()(a, b), 0);
    }
};

using value_equal_to            = basic_value_binary_predicate<value_compare,       std::equal_to<int>>;
using value_equal_to_icase      = basic_value_binary_predicate<value_compare_icase, std::equal_to<int>>;
using value_not_equal_to        = basic_value_binary_predicate<value_compare,       std::not_equal_to<int>>;
using value_not_equal_to_icase  = basic_value_binary_predicate<value_compare_icase, std::not_equal_to<int>>;
using value_less                = basic_value_binary_predicate<value_compare,       std::less<int>>;
using value_less_icase          = basic_value_binary_predicate<value_compare_icase, std::less<int>>;
using value_less_equal          = basic_value_binary_predicate<value_compare,       std::less_equal<int>>;
using value_less_equal_icase    = basic_value_binary_predicate<value_compare_icase, std::less_equal<int>>;
using value_greater             = basic_value_binary_predicate<value_compare,       std::greater<int>>;
using value_greater_icase       = basic_value_binary_predicate<value_compare_icase, std::greater<int>>;
using value_greater_equal       = basic_value_binary_predicate<value_compare,       std::greater_equal<int>>;
using value_greater_equal_icase = basic_value_binary_predicate<value_compare_icase, std::greater_equal<int>>;

/** \} **/

}

#endif/*__JSONV_FUNCTIONAL_HPP_INCLUDED__*/
