/** \file jsonv/string_view.hpp
 *  Pulls in an implementation of \c string_view.
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_STRING_VIEW_HPP_INCLUDED__
#define __JSONV_STRING_VIEW_HPP_INCLUDED__

#include <jsonv/config.hpp>

/** \def JSONV_STRING_VIEW_TYPE
 *  The type to use for \c jsonv::string_view. By default, this is \c jsonv::detail::string_view.
 *  
 *  \def JSONV_STRING_VIEW_INCLUDE
 *  The file to include to get the implementation for \c string_view. If you define \c JSONV_STRING_VIEW_TYPE, you must
 *  also define this.
 *  
 *  \def JSONV_STRING_VIEW_USE_STD
 *  Set this to 1 to use \c std::string_view as the backing type for \c jsonv::string_view.
 *  
 *  \def JSONV_STRING_VIEW_USE_BOOST
 *  Set this to 1 to use \c boost::string_ref as the backing type for \c jsonv::string_view.
**/
#ifndef JSONV_STRING_VIEW_TYPE
#   if defined(JSONV_STRING_VIEW_USE_STD) && JSONV_STRING_VIEW_USE_STD
#       define JSONV_STRING_VIEW_TYPE    std::string_view
#       define JSONV_STRING_VIEW_INCLUDE <string_view>
#   elif defined(JSONV_STRING_VIEW_USE_BOOST) && JSONV_STRING_VIEW_USE_BOOST
#       define JSONV_STRING_VIEW_TYPE    boost::string_ref
#       define JSONV_STRING_VIEW_INCLUDE <boost/utility/string_ref.hpp>
#   else
#       define JSONV_STRING_VIEW_TYPE    jsonv::detail::string_view
#       define JSONV_STRING_VIEW_INCLUDE <jsonv/detail/string_view.hpp>
#   endif
#endif

#include JSONV_STRING_VIEW_INCLUDE

namespace jsonv
{

/** A non-owning reference to a string.
 *  
 *  \see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3442.html
**/
using string_view = JSONV_STRING_VIEW_TYPE;

}

#endif/*__JSONV_STRING_REF_HPP_INCLUDED__*/
