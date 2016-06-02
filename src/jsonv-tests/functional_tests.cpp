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
#include <jsonv/functional.hpp>
#include <jsonv/value.hpp>
#include <jsonv/serialization_builder.hpp>

#include <algorithm>
#include <vector>

namespace jsonv_test
{

using namespace jsonv;

using string_list = std::vector<std::string>;

template <typename FJsonCmp, typename FStrCmp>
static void check_sort(const formats& fmts, string_list source, FJsonCmp json_cmp, FStrCmp str_cmp)
{
    value orig = to_json(source, fmts);
    if (source != extract<string_list>(orig, fmts))
        throw std::logic_error("Extraction or encoding is broken");
    
    std::sort(source.begin(),     source.end(),     str_cmp);
    std::sort(orig.begin_array(), orig.end_array(), json_cmp);
    
    if (source != extract<string_list>(orig, fmts))
        throw std::logic_error("Sorting did not produce identical results");
}

TEST(functional_sort_strings)
{
    formats fmts =
        formats::compose
        ({
            formats_builder()
                .register_container<std::vector<std::string>>(),
            formats::defaults()
        });
    std::vector<std::string> source = { "fire", "wind", "water", "earth", "heart" };
    
    check_sort(fmts, source, value_less(),          std::less<std::string>());
    check_sort(fmts, source, value_less_equal(),    std::less_equal<std::string>());
    check_sort(fmts, source, value_greater(),       std::greater<std::string>());
    check_sort(fmts, source, value_greater_equal(), std::greater_equal<std::string>());
}

}
