/** \file jsonv/coerce.hpp
 *  A \c jsonv::value has a number of \c as_X operators, which strictly performs a transformation to a C++ data type.
 *  However, sometimes when working with things like user input, you would like to be more free-form in what you accept
 *  as "valid."
 *  
 *  Copyright (c) 2014-2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_COERCE_HPP_INCLUDED__
#define __JSONV_COERCE_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/value.hpp>

#include <map>
#include <vector>

namespace jsonv
{

/** \addtogroup Coercion
 *  \{
 *  Looser conversion functions to behave more like ECMAScript.
**/

/** Can the given \c kind be converted \a from a kind \a to another?
 *  
 *  \returns \c true if the corresponding \c coerce_X function for the specified \a to will successfully return if given
 *           a \c value of the kind \a from; false if there is no such conversion (the \c coerce_X function might
 *           throw).
**/
JSONV_PUBLIC bool can_coerce(const kind& from, const kind& to);

/** Can the given \c value be converted \a from a kind \a to another?
 *  
 *  \note
 *  This is \e not only a convenience function! There is a special case for converting from a \c string into either a
 *  \c decimal or \c integer where the contents of the string must be considered. This function will look into the given
 *  \a from and see if it can successfully perfrom the coercion.
 *  
 *  \returns \c true if the corresponding \c coerce_X function for the specified \a to will successfully return if given
 *           the \c value \a from; false if there is no such conversion (the \c coerce_X function will throw).
**/
JSONV_PUBLIC bool can_coerce(const value& from, const kind& to);

/** Coerce \a from into a \c null. If \a from is not \c null, this will throw. It is not clear that there is a use for
 *  this beyond completeness.
 *  
 *  \returns \c nullptr if \a from has \c kind::null.
 *  \throws kind_error if \a from is not \c kind::null.
**/
JSONV_PUBLIC std::nullptr_t coerce_null(const value& from);

/** Coerce \a from into a \c map.
 *  
 *  \returns a map of the contents of \a from.
 *  \throws kind_error if \a from is not \c kind::object.
**/
JSONV_PUBLIC std::map<std::string, value> coerce_object(const value& from);

/** Coerce \a from into a \c vector.
 *  
 *  \returns a vector of the contents of \a from.
 *  \throws kind_error if \a from is not \c kind::array.
**/
JSONV_PUBLIC std::vector<value> coerce_array(const value& from);

/** Coerce \a from into an \c std::string. If \a from is already \c kind::string, the value is simply returned. If
 *  \a from is any other \c kind, the result will be the same as \c to_string.
**/
JSONV_PUBLIC std::string coerce_string(const value& from);

/** Coerce \a from into an integer. If \a from is a \c decimal lower than the minimum of \c std::int64_t or higher than
 *  the maximum of \c std::int64_t, it is clamped to the lowest or highest value, respectively.
 *  
 *  \returns
 *   \c kind is... | Rules
 *   ------------- | -------------------------------------------------
 *   \c null       | throws \c kind_error
 *   \c object     | throws \c kind_error
 *   \c array      | throws \c kind_error
 *   \c string     | \c parse(from.as_string()).as_integer()
 *   \c integer    | \c from.as_integer()
 *   \c decimal    | \c std::int64_t(from.as_decimal())
 *   \c boolean    | \c from.as_boolean() ? 1 : 0
**/
JSONV_PUBLIC std::int64_t coerce_integer(const value& from);

/** Coerce \a from into a \c double.
 *  
 *  \returns
 *   \c kind is... | Rules
 *   ------------- | -------------------------------------------------
 *   \c null       | throws \c kind_error
 *   \c object     | throws \c kind_error
 *   \c array      | throws \c kind_error
 *   \c string     | \c parse(from.as_string()).as_decimal()
 *   \c integer    | \c from.as_decimal()
 *   \c decimal    | \c from.as_decimal()
 *   \c boolean    | \c from.as_boolean() ? 1.0 : 0.0
**/
JSONV_PUBLIC double coerce_decimal(const value& from);

/** Coerce \a from into a \c bool. This follows the rules of Python's boolean coercion.
 *  
 *  \returns
 *   \c kind is... | Rules
 *   ------------- | --------------------------------------------------
 *   \c null       | \c false
 *   \c object     | \c !from.empty()
 *   \c array      | \c !from.empty()
 *   \c string     | \c !from.empty() (even if the value is \c "false")
 *   \c integer    | \c from != 0
 *   \c decimal    | \c from != 0.0
 *   \c boolean    | \c from.as_boolean()
**/
JSONV_PUBLIC bool coerce_boolean(const value& from);

/** Combines \a a and \a b in any way possible. The result kind is \e usually based on the kind of \a a and loosely
 *  follows what ECMAScript does when you call \c + on two values (sort of). If you are looking for "predictable", this
 *  is not the function for you. If you are looking for convenience, this is it.
**/
JSONV_PUBLIC value coerce_merge(value a, value b);

/** \} **/

}

#endif/*__JSONV_COERCE_HPP_INCLUDED__*/
