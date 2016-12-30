/** \file
 *  
 *  Copyright (c) 2012-2016 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <algorithm>
#include <cassert>
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <jsonv/value.hpp>
#include <jsonv/parse.hpp>

#include "filesystem_util.hpp"
#include "test.hpp"

TEST(demo)
{
    std::string src = "{ \"blazing\": [ 3, \"\\\"\\n\", 4.5, 5.123, 4.10921e19 ], "
                      "  \"text\": [ 1, 2, 3, 4, \t\"something\"],  "
                      "  \"we call him \\\"empty array\\\"\": [], "
                      "  \"we call him \\\"empty object\\\"\": {}, "
                      "  \"unicode\" :\"\\uface\""
                      "}";
    jsonv::value parsed = jsonv::parse(src);
    std::cout << parsed;
}

int main(int argc, char** argv)
{
    std::string filter;
    if (argc == 2)
        filter = argv[1];
    
    int fail_count = 0;
    for (auto test : jsonv_test::get_unit_tests())
    {
        bool shouldrun = filter.empty()
                      || test->name().find(filter) != std::string::npos;
        if (shouldrun && !test->run())
            ++fail_count;
    }
    #ifdef _MSC_VER
    // Visual Studio doesn't seem to treat non-zero exits as an error...so we'll just throw an exception.
    if (fail_count > 0)
        throw std::runtime_error("Not all unit tests passed...");
    #endif
    return fail_count;
}
