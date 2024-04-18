/*  
 * File:   split_string.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on February 20, 2017, 11:19 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __RTB_COMMON_SPLIT_STRING_HPP__
#define	__RTB_COMMON_SPLIT_STRING_HPP__

#include <vector>
#include <string>
#include <iterator>
#include <cstring>

namespace vanilla { namespace common {
    
template<typename StringView>
void split_string( std::vector<StringView> &ret, 
                   const std::string &s, 
                   const char *delims) {
    char const* begin = s.c_str();
    char const* end = strpbrk((*begin == *delims) ? begin : begin + 1, delims);
    for (; end != NULL; end = strpbrk(begin, delims)) {
        ret.emplace_back(begin, std::distance(begin,end));
        begin = ++end;
    }
    
    if ( begin != &*s.end() ) {
        ret.emplace_back(begin, std::distance(begin, &*s.end())) ;
    }
}

}}

#endif	/* __RTB_COMMON_SPLIT_STRING_HPP__ */

