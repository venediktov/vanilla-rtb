/** \file
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include "test.hpp"

#include <jsonv/tokenizer.hpp>

#include <random>
#include <sstream>

namespace jsonv_test
{

using namespace jsonv;

TEST(token_kind_streaming)
{
    ensure_eq(to_string(token_kind::boolean), "boolean");
    ensure_eq(to_string(token_kind::boolean | token_kind::string), "boolean|string");
    ensure_eq(to_string(token_kind::boolean | token_kind::parse_error_indicator), "parse_error(boolean)");
    ensure_eq(to_string(token_kind::number | static_cast<token_kind>(0x4000)), "number|0x4000");
}

TEST(token_kind_streaming_random)
{
    // create a bunch of (most likely invalid) token_kind values and to_string them...this sort of checks that we
    // never infinitely loop in the output formatter
    std::random_device prng;
    for (std::size_t x = 0; x < 1000; ++x)
    {
        token_kind tok = static_cast<token_kind>(prng());
        to_string(tok);
    }
}

TEST(tokenizer_single_boolean)
{
    std::string input = "true";
    std::istringstream istream(input);
    tokenizer tokens(istream);
    ensure(tokens.next());
    auto found = tokens.current();
    ensure_eq(found.kind, token_kind::boolean);
    ensure_eq(found.text, "true");
}

TEST(tokenizer_string)
{
    std::string input = "\"true\"";
    std::istringstream istream(input);
    tokenizer tokens(istream);
    ensure(tokens.next());
    auto found = tokens.current();
    ensure_eq(found.kind, token_kind::string);
    ensure_eq(found.text, "\"true\"");
}

}
