/** \file
 *  Internal-only header for regex.
 *  
 *  Copyright (c) 2016 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_REGEX_HPP_INCLUDED__
#define __JSONV_DETAIL_REGEX_HPP_INCLUDED__

/** \def JSONV_REGEX_INCLUDE
 *  Controls the regular expression engine to use. By default, this will use the C++ Standard Library implementation.
 *  GCC versions below 4.8 will happy compile regular expressions, but will fail at runtime. If using GCC under 4.9, it
 *  is recommended that you set \c JSONV_REGEX_USE_BOOST to \c 1.
 *  
 *  \def JSONV_REGEX_NAMESPACE
 *  The regular expression namespace to use. By default, this is \c std.
 *  
 *  \def JSONV_REGEX_USE_BOOST
 *  Use Boost as the regular expression engine. Sets \c JSONV_REGEX_INCLUDE to \c <boost/regex.hpp> and
 *  \c JSONV_REGEX_NAMESPACE to \c boost.
**/
#ifndef JSONV_REGEX_INCLUDE
#   if defined(JSONV_REGEX_USE_BOOST) && JSONV_REGEX_USE_BOOST
#       define JSONV_REGEX_INCLUDE   <boost/regex.hpp>
#       define JSONV_REGEX_NAMESPACE boost
#   else
#       define JSONV_REGEX_INCLUDE   <regex>
#       define JSONV_REGEX_NAMESPACE std
#   endif
#elif !defined(JSONV_REGEX_NAMESPACE)
#   error "JSONV_REGEX_NAMESPACE is unset, but JSONV_REGEX_INCLUDE is. You must set both."
#elif defined(JSONV_REGEX_NAMESPACE)
#   error "JSONV_REGEX_INCLUDE is unset, but JSONV_REGEX_NAMESPACE is. You must set both."
#endif

#include JSONV_REGEX_INCLUDE

namespace jsonv
{
namespace detail
{

namespace regex = JSONV_REGEX_NAMESPACE;

}
}

#endif/*__JSONV_DETAIL_REGEX_HPP_INCLUDED__*/
