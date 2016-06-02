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
#include <jsonv/functional.hpp>
#include <jsonv/value.hpp>

#include <cctype>

#include "detail.hpp"

namespace jsonv
{

int value_compare::operator()(const value& a, const value& b) const
{
    return compare(a, b);
}

int value_compare_icase::operator()(const value& a, const value& b) const
{
    return compare_icase(a, b);
}

}
