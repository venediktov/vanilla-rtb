/** \file jsonv/algorithm.hpp
 *  A collection of algorithms a la `&lt;algorithm&gt;`.
 *  
 *  Copyright (c) 2014-2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_ALGORITHM_HPP_INCLUDED__
#define __JSONV_ALGORITHM_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/value.hpp>

#include <cmath>
#include <functional>
#include <limits>

namespace jsonv
{

class path;

/** \addtogroup Algorithm
 *  \{
 *  A collection of useful free functions a la \c <algorithm>.
**/

/** Traits describing how to perform various aspects of comparison. This implementation for comparison is strict and is
 *  ultimately the one used by \c value::compare.
 *  
 *  \see compare
**/
struct JSONV_PUBLIC compare_traits
{
    /** Compare two kinds \a a and \a b. This should return 0 if the types are the same or if they are directly
     *  comparable (such as \c kind::integer and \c kind::decimal) -- if you return 0 for non-comparable types, you risk
     *  getting a \c kind_error thrown.
    **/
    static int compare_kinds(kind a, kind b)
    {
        int va = kindval(a);
        int vb = kindval(b);
        return va == vb ? 0 : va < vb ? -1 : 1;
    }
    
    /** Compare two boolean values. **/
    static int compare_booleans(bool a, bool b)
    {
        return a == b ?  0
             : a      ?  1
             :          -1;
    }
    
    /** Compare two integer values. **/
    static int compare_integers(std::int64_t a, std::int64_t b)
    {
        return a == b ?  0
             : a <  b ? -1
             :           1;
    }

    /** Compare two decimal values. **/
    static int compare_decimals(double a, double b)
    {
        return (std::abs(a - b) < (std::numeric_limits<double>::denorm_min() * 10.0)) ?  0
             : (a < b)                                                                ? -1
             :                                                                           1;
    }
    
    /** Compare two string values. **/
    static int compare_strings(const std::string& a, const std::string& b)
    {
        return a.compare(b);
    }
    
    /** Compare two strings used for the keys of objects. **/
    static int compare_object_keys(const std::string& a, const std::string& b)
    {
        return a.compare(b);
    }
    
    /** Compare two objects \e before comparing the values. The \c compare function will only check the contents of an
     *  object if this function returns 0.
    **/
    static int compare_objects_meta(const value&, const value&)
    {
        return 0;
    }
    
private:
    static int kindval(kind k)
    {
        switch (k)
        {
        case jsonv::kind::null:
            return 0;
        case jsonv::kind::boolean:
            return 1;
        case jsonv::kind::integer:
        case jsonv::kind::decimal:
            return 2;
        case jsonv::kind::string:
            return 3;
        case jsonv::kind::array:
            return 4;
        case jsonv::kind::object:
            return 5;
        default:
            return -1;
        }
    }
};

/** Compare the values \a a and \a b using the comparison \a traits.
 *  
 *  \tparam TCompareTraits A type which should be compatible with the public signatures on the \c compare_traits class.
**/
template <typename TCompareTraits>
int compare(const value& a, const value& b, const TCompareTraits& traits)
{
    if (&a == &b)
        return 0;
    
    if (int kindcmp = traits.compare_kinds(a.kind(), b.kind()))
        return kindcmp;
    
    switch (a.kind())
    {
    case jsonv::kind::null:
        return 0;
    case jsonv::kind::boolean:
        return traits.compare_booleans(a.as_boolean(), b.as_boolean());
    case jsonv::kind::integer:
        // b might be a decimal type, but if they are both integers, compare directly
        if (b.kind() == jsonv::kind::integer)
            return traits.compare_integers(a.as_integer(), b.as_integer());
        // fall through
    case jsonv::kind::decimal:
        return traits.compare_decimals(a.as_decimal(), b.as_decimal());
    case jsonv::kind::string:
        return traits.compare_strings(a.as_string(), b.as_string());
    case jsonv::kind::array:
    {
        auto aiter = a.begin_array();
        auto biter = b.begin_array();
        for ( ; aiter != a.end_array() && biter != b.end_array(); ++aiter, ++biter)
            if (int cmp = compare(*aiter, *biter, traits))
                return cmp;
        return aiter == a.end_array() ? biter == b.end_array() ? 0 : -1
                                      : 1;
    }
    case jsonv::kind::object:
    {
        if (int objmetacmp = traits.compare_objects_meta(a, b))
            return objmetacmp;
        
        auto aiter = a.begin_object();
        auto biter = b.begin_object();
        for ( ; aiter != a.end_object() && biter != b.end_object(); ++aiter, ++biter)
        {
            if (int cmp = traits.compare_object_keys(aiter->first, biter->first))
                return cmp;
            if (int cmp = compare(aiter->second, biter->second, traits))
                return cmp;
        }
        return aiter == a.end_object() ? biter == b.end_object() ? 0 : -1
                                       : 1;
    }
    default:
        return -1;
    }
}

/** Compare the values \a a and \a b with strict comparison traits.
 *  
 *  \see value::compare
 *  \see compare_icase
**/
JSONV_PUBLIC int compare(const value& a, const value& b);

/** Compare the values \a a and \a b, but use case-insensitive matching on \c kind::string values. This does \e not use
 *  case-insensitive matching on the keys of objects!
 *  
 *  \see compare
**/
JSONV_PUBLIC int compare_icase(const value& a, const value& b);

/** Run a function over the values in the \a input. The behavior of this function is different, depending on the \c kind
 *  of \a input. For scalar kinds (\c kind::integer, \c kind::null, etc), \a func is called once with the value. If
 *  \a input is \c kind::array, \c func is called for every value in the array and the output will be an array with each
 *  element transformed by \a func. If \a input is \c kind::object, the result will be an object with each key
 *  transformed by \a func.
 *  
 *  \param func The function to apply to the element or elements of \a input.
 *  \param input The value to transform.
**/
JSONV_PUBLIC value map(const std::function<value (const value&)>& func,
                       const value&                               input
                      );

/** Run a function over the values in the \a input. The behavior of this function is different, depending on the \c kind
 *  of \a input. For scalar kinds (\c kind::integer, \c kind::null, etc), \a func is called once with the value. If
 *  \a input is \c kind::array, \c func is called for every value in the array and the output will be an array with each
 *  element transformed by \a func. If \a input is \c kind::object, the result will be an object with each key
 *  transformed by \a func.
 *  
 *  \param func The function to apply to the element or elements of \a input.
 *  \param input The value to transform.
 *  
 *  \note
 *  This version of \c map provides only a basic exception-safety guarantee. If an exception is thrown while
 *  transforming a non-scalar \c kind, there is no rollback action, so \a input is left in a usable, but
 *  \e unpredictable state. If you need a strong exception guarantee, use the version of \c map that takes a constant
 *  reference to a \c value.
**/
JSONV_PUBLIC value map(const std::function<value (value)>& func,
                       value&&                             input
                      );

/** Recursively walk the provided \a tree and call \a func for each item in the tree.
 *  
 *  \param tree The JSON value to traverse.
 *  \param func The function to call for each element in the tree.
 *  \param base_path The path to prepend to each output path to \a func. This can be useful if beginning traversal from
 *                   inside of some JSON structure.
 *  \param leafs_only If true, call \a func only when the current path is a "leaf" value (\c string, \c integer,
 *                    \c decimal, \c boolean, or \c null \e or an empty \c array or \c object); if false, call \a func
 *                    for all entries in the tree.
**/
JSONV_PUBLIC void traverse(const value&                                           tree,
                           const std::function<void (const path&, const value&)>& func,
                           const path&                                            base_path,
                           bool                                                   leafs_only = false
                          );

/** Recursively walk the provided \a tree and call \a func for each item in the tree.
 *  
 *  \param tree The JSON value to traverse.
 *  \param func The function to call for each element in the tree.
 *  \param leafs_only If true, call \a func only when the current path is a "leaf" value (\c string, \c integer,
 *                    \c decimal, \c boolean, or \c null \e or an empty \c array or \c object); if false, call \a func
 *                    for all entries in the tree.
**/
JSONV_PUBLIC void traverse(const value&                                           tree,
                           const std::function<void (const path&, const value&)>& func,
                           bool                                                   leafs_only = false
                          );

/** \} **/

}

#endif/*__JSONV_ALGORITHM_HPP_INCLUDED__*/
