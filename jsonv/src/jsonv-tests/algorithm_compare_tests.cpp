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

TEST(compare_icase_sames)
{
    ensure_eq(compare_icase("A", "a"), 0);
    ensure_eq(compare_icase("a", "A"), 0);
    ensure_eq(compare_icase("a", "a"), 0);
    ensure_eq(compare_icase("A", "A"), 0);
}

TEST(compare_icase_diffs)
{
    ensure_lt(compare_icase("A", "b"), 0);
    ensure_lt(compare_icase("a", "B"), 0);
    ensure_gt(compare_icase("b", "a"), 0);
    ensure_gt(compare_icase("B", "A"), 0);
}

TEST(compare_icase_empty)
{
    ensure_eq(compare_icase("",  ""), 0);
    ensure_gt(compare_icase("a", ""), 0);
    ensure_lt(compare_icase("", "a"), 0);
}

}
