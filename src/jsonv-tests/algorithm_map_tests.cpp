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

#include <jsonv/algorithm.hpp>
#include <jsonv/value.hpp>

namespace jsonv_test
{

using namespace jsonv;

TEST(map_scalar)
{
    value init = 2;
    value result = init.map([] (const value& x) { return x.as_integer() * 2; });
    ensure_eq(init, 2);
    ensure_eq(result, 4);
}

TEST(map_array)
{
    value init = array({ 1, 2, 3 });
    value result = init.map([] (const value& x) { return x.as_integer() * 2; });
    ensure_eq(init,   array({ 1, 2, 3 }));
    ensure_eq(result, array({ 2, 4, 6 }));
}

TEST(map_object)
{
    value init = object({ { "one", 1 }, { "two", 2 } });
    value result = init.map([] (const value& x) { return x.as_integer() * 2; });
    ensure_eq(init,   object({ { "one", 1 }, { "two", 2 } }));
    ensure_eq(result, object({ { "one", 2 }, { "two", 4 } }));
}

TEST(map_scalar_rvalue)
{
    value init = 2;
    value result = std::move(init).map([] (const value& x) { return x.as_integer() * 2; });
    ensure_eq(init,   value(null));
    ensure_eq(result, 4);
}

TEST(map_array_rvalue)
{
    value init = array({ 1, 2, 3 });
    value result = std::move(init).map([] (const value& x) { return x.as_integer() * 2; });
    ensure_eq(init,   value(null));
    ensure_eq(result, array({ 2, 4, 6 }));
}

TEST(map_object_rvalue)
{
    value init = object({ { "one", 1 }, { "two", 2 } });
    value result = std::move(init).map([] (const value& x) { return x.as_integer() * 2; });
    ensure_eq(init, value(null));
    ensure_eq(result, object({ { "one", 2 }, { "two", 4 } }));
}

}
