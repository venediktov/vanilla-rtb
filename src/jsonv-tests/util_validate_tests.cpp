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

#include <jsonv/util.hpp>
#include <jsonv/value.hpp>

#include <cmath>

namespace jsonv_test
{

using namespace jsonv;

TEST(validate_non_finite_number)
{
    value src = object({ { "a", 1 },
                         { "b", 2.5 },
                         { "c", array({ "hi", std::nan(""), 3.0 }) }
                       }
                      );
    ensure_throws(validation_error, validate(src));
    try
    {
        validate(src);
    }
    catch (const validation_error& err)
    {
        ensure_eq(err.error_code(), validation_error::code::non_finite_number);
        ensure_eq(err.path(), path::create(".c[1]"));
        ensure_eq(err.value().kind(), kind::decimal);
        ensure(std::isnan(err.value().as_decimal()));
    }
}

}
