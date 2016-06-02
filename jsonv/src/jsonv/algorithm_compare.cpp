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

#include <cctype>

namespace jsonv
{

int compare(const value& a, const value& b)
{
    return compare(a, b, compare_traits());
}

struct compare_traits_icase :
        public compare_traits
{
    /** Compares strings a and b in a case-insensitive manner. It is not UTF-8 aware and I am not sure it needs to be. **/
    static int compare_strings(const std::string& a, const std::string& b)
    {
        using std::begin;
        using std::end;
        
        auto aiter = begin(a);
        auto biter = begin(b);
        
        for ( ; aiter != end(a) && biter != end(b); ++aiter, ++biter)
        {
            auto aa = std::tolower(*aiter);
            auto bb = std::tolower(*biter);
            if (aa == bb)
                continue;
            else if (aa < bb)
                return -1;
            else
                return 1;
        }
        
        return aiter == end(a) ? biter == end(b) ? 0 : -1
                               : 1;
    }
};

int compare_icase(const value& a, const value& b)
{
    return compare(a, b, compare_traits_icase());
}

}
