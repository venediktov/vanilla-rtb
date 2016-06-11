/** \file jsonv/optional.hpp
 *  Pulls in an implementation of \c optional.
 *  
 *  Copyright (c) 2016 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_OPTIONAL_HPP_INCLUDED__
#define __JSONV_OPTIONAL_HPP_INCLUDED__

#include <jsonv/config.hpp>

#ifndef JSONV_OPTIONAL_USE_STD
#   define JSONV_OPTIONAL_USE_STD 0
#endif

#ifndef JSONV_OPTIONAL_USE_STD_EXPERIMENTAL
#   define JSONV_OPTIONAL_USE_STD_EXPERIMENTAL 0
#endif

#ifndef JSONV_OPTIONAL_USE_BOOST
#   define JSONV_OPTIONAL_USE_BOOST 0
#endif

// Attempt to guess between JSONV_OPTIONAL_USE_STD and JSONV_OPTIONAL_USE_STD_EXPERIMENTAL based on C++ language version
#if !defined(JSONV_OPTIONAL_TYPE) && !JSONV_OPTIONAL_USE_STD && !JSONV_OPTIONAL_USE_STD_EXPERIMENTAL && !JSONV_OPTIONAL_USE_BOOST
#   if __cplusplus > 201700L
#       undef  JSONV_OPTIONAL_USE_STD
#       define JSONV_OPTIONAL_USE_STD 1
#   else
#       undef  JSONV_OPTIONAL_USE_STD_EXPERIMENTAL
#       define JSONV_OPTIONAL_USE_STD_EXPERIMENTAL 1
#   endif
#endif

/** \def JSONV_OPTIONAL_TYPE
 *  The type to use for \c jsonv::optional. By default, this is \c std::optional or \c std::experimental::optional,
 *  depending on the C++ language version you are compiling with.
 *  
 *  \def JSONV_NULLOPT_VALUE
 *  The value to use for \c jsonv::nullopt By default, this is \c std::nullopt or \c std::experimental::nullopt (by the
 *  same rules as \c JSONV_OPTIONAL_TYPE). If you define \c JSONV_OPTIONAL_TYPE, you must also define this.
 *  
 *  \def JSONV_OPTIONAL_INCLUDE
 *  The file to include to get the implementation for \c optional. If you define \c JSONV_OPTIONAL_TYPE, you must
 *  also define this.
 *  
 *  \def JSONV_OPTIONAL_USE_STD
 *  Set this to 1 to use \c std::optional as the backing type for \c jsonv::optional.
 *  
 *  \def JSONV_OPTIONAL_USE_STD_EXPERIMENTAL
 *  Set this to 1 to use \c std::experimental::optional as the backing type for \c jsonv::optional.
 *  
 *  \def JSONV_OPTIONAL_USE_BOOST
 *  Set this to 1 to use \c boost::optional as the backing type for \c jsonv::optional.
**/
#ifndef JSONV_OPTIONAL_TYPE
#   if defined(JSONV_OPTIONAL_USE_STD_EXPERIMENTAL) && JSONV_OPTIONAL_USE_STD_EXPERIMENTAL
#       define JSONV_OPTIONAL_TYPE    std::experimental::optional
#       define JSONV_NULLOPT_VALUE    std::experimental::nullopt
#       define JSONV_OPTIONAL_INCLUDE <experimental/optional>
#   elif defined(JSONV_OPTIONAL_USE_BOOST) && JSONV_OPTIONAL_USE_BOOST
#       define JSONV_OPTIONAL_TYPE    boost::optional
#       define JSONV_NULLOPT_VALUE    boost::none
#       define JSONV_OPTIONAL_INCLUDE <boost/optional.hpp>
#   else
#       define JSONV_OPTIONAL_TYPE    std::optional
#       define JSONV_NULLOPT_VALUE    std::nullopt
#       define JSONV_OPTIONAL_INCLUDE <optional>
#   endif
#endif

#include JSONV_OPTIONAL_INCLUDE

namespace jsonv
{

/** Represents a value that may or may not be present.
 *  
 *  \see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3442.html
**/
template <typename T>
using optional = JSONV_OPTIONAL_TYPE<T>;

namespace
{

constexpr auto& nullopt JSONV_UNUSED = JSONV_NULLOPT_VALUE;

}

}

#endif/*__JSONV_OPTIONAL_HPP_INCLUDED__*/
