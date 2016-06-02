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
#include "test.hpp"

#include <jsonv/value.hpp>
#include <jsonv/detail.hpp>

namespace jsonv_test
{

using namespace jsonv;

TEST(kind_serialization)
{
    ensure_eq(to_string(kind::array),   "array");
    ensure_eq(to_string(kind::object),  "object");
    ensure_eq(to_string(kind::string),  "string");
    ensure_eq(to_string(kind::integer), "integer");
    ensure_eq(to_string(kind::decimal), "decimal");
    ensure_eq(to_string(kind::boolean), "boolean");
    ensure_eq(to_string(kind::null),    "null");
    
    // all we care about here is that it doesn't assert
    to_string(static_cast<kind>(~0));
}

TEST(kind_valid)
{
    ensure(!kind_valid(static_cast<kind>(~0)));
}

TEST(check_type_valid)
{
    check_type({ kind::array, kind::object, kind::null }, kind::object);
}

TEST(check_type_invalid)
{
    ensure_throws(kind_error, check_type({ kind::array, kind::object, kind::null }, kind::integer));
}

}
