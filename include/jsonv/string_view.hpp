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

#ifdef JSONV_STRING_VIEW_INCLUDE
#   include JSONV_STRING_VIEW_INCLUDE
#endif

namespace jsonv
{

/** A non-owning reference to a string.
 *  
 *  \see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3442.html
**/
using string_view = JSONV_STRING_VIEW_TYPE;

}

#endif/*__JSONV_STRING_REF_HPP_INCLUDED__*/
