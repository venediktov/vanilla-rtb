/** \file
 *  
 *  Copyright (c) 2012 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/value.hpp>
#include <jsonv/parse.hpp>

#include "test.hpp"

TEST(parse_empty_string)
{
    jsonv::value val = jsonv::parse("\"\"");
    const std::string& str = val.as_string();
    ensure(str == "");
}

TEST(parse_single_backslash)
{
    jsonv::value val = jsonv::parse("\"\\\\\""); // "\\"
    const std::string& str = val.as_string();
    ensure(str == "\\");
    ensure_eq(1, val.size());
}

TEST(parse_bogus_string_throws)
{
    // Specify a Unicode escape that isn't long enough and doesn't use hex.
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\\uTT\""));
}

TEST(parse_strict_string_unprintables)
{
    auto options = jsonv::parse_options::create_strict();
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\t\"",   options));
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\b\"",   options));
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\f\"",   options));
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\n\"",   options));
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\r\"",   options));
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\x01\"", options));
}

TEST(string_comparisons)
{
    using namespace jsonv;
    
    value fire  = "fire";
    value wind  = "wind";
    value water = "water";
    value earth = "earth";
    value heart = "heart";
    
    ensure_eq(fire, fire);
    ensure_lt(fire, wind);
    ensure_lt(fire, water);
    ensure_gt(fire, earth);
    ensure_lt(fire, heart);
    
    ensure_gt(wind, fire);
    ensure_eq(wind, wind);
    ensure_gt(wind, water);
    ensure_gt(wind, earth);
    ensure_gt(wind, heart);
    
    ensure_gt(water, fire);
    ensure_lt(water, wind);
    ensure_eq(water, water);
    ensure_gt(water, earth);
    ensure_gt(water, heart);
    
    ensure_lt(earth, fire);
    ensure_lt(earth, wind);
    ensure_lt(earth, water);
    ensure_eq(earth, earth);
    ensure_lt(earth, heart);
    
    ensure_gt(heart, fire);
    ensure_lt(heart, wind);
    ensure_lt(heart, water);
    ensure_gt(heart, earth);
    ensure_eq(heart, heart);
}

TEST(wide_strings)
{
    using namespace jsonv;
    
    value narrow =  "some basic text";
    value wide   = L"some basic text";
    
    ensure_eq(narrow, wide);
    ensure(narrow.as_wstring() == wide.as_wstring());
}

TEST(string_view_construction)
{
    using namespace jsonv;

    value cp("Bob");
    value sv(cp.as_string_view());

    ensure_eq(cp, sv);
}
