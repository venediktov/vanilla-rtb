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

#include <jsonv/array.hpp>
#include <jsonv/parse.hpp>
#include <jsonv/object.hpp>
#include <jsonv/tokenizer.hpp>

#include <iostream>

using namespace jsonv;

static const value simple_obj = object({ { "foo", 4 },
                                         { "bar", array({ 2, 3, 4, "5" }) },
                                         { "raz", object() }
                                       });

#define TEST_PARSE(name) TEST(parse_ ## name)

TEST_PARSE(object_simple_no_spaces)
{
    value result = parse("{\"foo\":4,\"bar\":[2,3,4,\"5\"],\"raz\":{}}");
    ensure_eq(simple_obj, result);
}

TEST_PARSE(object_simple_no_newlines)
{
    value result = parse("{\"foo\": 4, \"raz\": {   }, \"bar\": [ 2, 3, 4, \"5\"]}");
    ensure_eq(simple_obj, result);
}

TEST_PARSE(object_simple_spaces_and_tabs)
{
    value result = parse("           {    \t       \"foo\" :                 \t            4  , "
                         " \"bar\"                                :               [          \t         2\t,"
                         " 3\t\t\t,\t4                ,                    \"5\"             ],"
                         "         \t\"raz\"      \t: {                                                   }"
                         " \t\t}            \t");
    ensure_eq(simple_obj, result);
}

TEST_PARSE(object_simple_newlines)
{
    value result = parse(R"({
    "foo":4,
    "bar":[2,3,4,"5"],
    "raz":{}
})");
    ensure_eq(simple_obj, result);
}

TEST_PARSE(object_simple_general_havoc)
{
    value result = parse(R"(
        {
                                "foo"
:
                4,"raz"
                                                                        :
{
    
    
    
    
        },"bar"       :
              [
               2,
                         3,4,
                      "5"
]}
        
        
    )");
    ensure_eq(simple_obj, result);
}

TEST_PARSE(object_nested_single)
{
    value result = parse(R"({"a": {"b": 10}, "c":25})");
    value expected = object({ { "a", object({ { "b", 10 } }) },
                              { "c", 25 }
                           });
    ensure_eq(expected, result);
}

TEST_PARSE(object_empties_in_array)
{
    value result = parse(R"({"a": {"b": 10}, "c": 23.9, "d": [{"e": {}, "f": 41.4, "g": null, "h": 5}, {"i":null}]})");
    value expected = object({ { "a", object({ { "b", 10 } }) },
                              { "c", 23.9 },
                              { "d", array({ object({ { "e", object() },
                                                      { "f", 41.4 },
                                                      { "g", null },
                                                      { "h", 5 },
                                                   }),
                                             object({ { "i", null } })
                                          })
                              }
                           });
    ensure_eq(expected, result);
}

TEST_PARSE(empty_object_char_ptr_range)
{
    const char buff[] = "{}";
    value result = parse(buff + 0, buff + sizeof buff);
    value expected = object();
    ensure_eq(expected, result);
}

TEST_PARSE(null)
{
    value result = parse("null");
    value expected = value(null);
    ensure_eq(expected, result);
}

TEST_PARSE(null_in_arr)
{
    value result = parse("[null,4]");
    value expected = array({ null, 4 });
    ensure_eq(expected, result);
}

TEST_PARSE(null_in_obj)
{
    value result = parse(R"({"a": null})");
    value expected = object({ { "a", null } });
    ensure_eq(expected, result);
}

TEST_PARSE(malformed_bools)
{
    ensure_throws(parse_error, parse("truish"));
    ensure_throws(parse_error, parse("tru"));
    ensure_throws(parse_error, parse("falsy"));
}

TEST_PARSE(malformed_nulls)
{
    ensure_throws(parse_error, parse("nul"));
}

TEST_PARSE(object_in_array)
{
    value result = parse(R"({"a": null, "b": {"c": 1, "d": "e", "f": null, "g": 2, )"
                         R"("h": [{"i": 3, "j": null, "k": "l", "m": 4, "n": "o", "p": "q", "r": 5}, )"
                         R"({"s": 6, "t": 7, "u": null}, {"v": "w"}]}})"
                        );
    value expected = object({ { "a", null },
                              { "b", object({ { "c", 1 },
                                              { "d", "e" },
                                              { "f", null },
                                              { "g", 2 },
                                              { "h", array({ object({ { "i", 3 },
                                                                      { "j", null },
                                                                      { "k", "l" },
                                                                      { "m", 4 },
                                                                      { "n", "o" },
                                                                      { "p", "q" },
                                                                      { "r", 5 },
                                                                   }),
                                                             object({ { "s", 6 },
                                                                      { "t", 7 },
                                                                      { "u", null },
                                                                   }),
                                                             object({ { "v", "w" } })
                                                          })
                                              },
                                           })
                              }
                           });
    ensure_eq(expected, result);
}

TEST_PARSE(malformed_decimal)
{
    ensure_throws(jsonv::parse_error, parse("123.456.789"));
}

TEST_PARSE(malformed_decimal_collect_all)
{
    auto options = jsonv::parse_options()
                       .failure_mode(jsonv::parse_options::on_error::collect_all);
    ensure_throws(jsonv::parse_error, parse("123.456.789", options));
}

TEST_PARSE(malformed_decimal_in_object)
{
    ensure_throws(jsonv::parse_error, parse(R"({"x": 123.456.789 })"));
}

TEST_PARSE(malformed_decimal_ignore)
{
    auto options = jsonv::parse_options()
                       .failure_mode(jsonv::parse_options::on_error::ignore);
    // Could potentially check that the result is still a decimal, but the result is undefined.
    parse("123.456.789", options);
}

TEST_PARSE(malformed_string_unterminated)
{
    ensure_throws(jsonv::parse_error, parse(R"("abc)"));
    ensure_throws(jsonv::parse_error, parse(R"(")"));
}

TEST_PARSE(malformed_boolean)
{
    ensure_throws(jsonv::parse_error, parse("try"));
}

TEST_PARSE(option_complete_parse_false)
{
    auto options = jsonv::parse_options()
                         .complete_parse(false);
    std::string input = R"({ "x": [4, 3, 5] })";
    jsonv::value expected = parse(input);
    std::istringstream istream(input + input + input + input);
    jsonv::tokenizer tokens(istream);
    for (std::size_t x = 0; x < 4; ++x)
    {
        jsonv::value entry = jsonv::parse(tokens, options);
        ensure_eq(expected, entry);
    }
}

TEST_PARSE(partial_array)
{
    try
    {
        parse_options options = parse_options()
                                .failure_mode(parse_options::on_error::collect_all)
                                .max_failures(1);
        parse("[1, 2, bogus]", options);
        ensure(false);
    }
    catch (const jsonv::parse_error& err)
    {
        // just check that we can...
        to_string(err);
        to_string(err.problems().at(0));
        value expected = array({ 1, 2, null });
        ensure_eq(expected, err.partial_result());
    }
}

TEST_PARSE(depth)
{
    std::string src = R"({"a": null, "b": [{}, 3, 4.5, false, [[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]})";
    // this isn't all that useful -- we just want to ensure that the normal src parses
    parse(src);
    ensure_throws(parse_error, parse(src, parse_options::create_strict()));
}

TEST_PARSE(literal)
{
    value v = "[1, 2, 3, 4]"_json;
    ensure_eq(array({ 1, 2, 3, 4 }), v);
}

TEST_PARSE(comments_invalid_leading_slash_then_bogus)
{
    ensure_throws(parse_error, parse("{}/1"));
}

TEST_PARSE(malformed_comment_complete)
{
    ensure_throws(parse_error, parse("/1"));
}

TEST_PARSE(malformed_comment_in_object)
{
    ensure_throws(parse_error, parse(R"({"a": ////////"b"})"));
}

TEST_PARSE(comment_in_object)
{
    value val = parse(R"({"a": /* yo */"b"})");
    ensure_eq(object({ { "a", "b" } }), val);
}

TEST_PARSE(comment_in_array)
{
    value val = parse(R"(["a", /* yo */"b"])");
    ensure_eq(array({ "a", "b" }), val);
}

TEST_PARSE(invalid_utf8_input)
{
    ensure_throws(jsonv::parse_error, jsonv::parse("\"\xe4\""));
}
