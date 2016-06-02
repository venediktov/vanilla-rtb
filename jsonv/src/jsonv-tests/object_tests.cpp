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
#include "test.hpp"

#include <jsonv/array.hpp>
#include <jsonv/object.hpp>
#include <jsonv/parse.hpp>

#include <string>
#include <utility>

TEST(object)
{
    jsonv::value obj = jsonv::object();
    obj["hi"] = false;
    ensure(obj["hi"].as_boolean() == false);
    obj["yay"] = jsonv::array({ "Hello", "to", "the", "world" });
    ensure(obj["hi"].as_boolean() == false);
    ensure(obj["yay"].size() == 4);
    ensure(obj.size() == 2);
}

TEST(object_view_iter_assign)
{
    using namespace jsonv;
    
    value obj = object({ { "foo", 5 }, { "bar", "wat" } });
    value found = object({ { "foo", false }, { "bar", false } });
    ensure(obj.size() == 2);
    
    for (auto iter = obj.begin_object(); iter != obj.end_object(); ++iter)
    {
        value::object_iterator fiter;
        fiter = found.find(iter->first);
        ensure(!fiter->second.as_boolean());
        fiter->second = true;
    }
    
    for (auto iter = found.begin_object(); iter != found.end_object(); ++iter)
        ensure(iter->second.as_boolean());
}

TEST(object_view_reverse_iter)
{
    using namespace jsonv;
    
    value obj = object({ { "a", 1 }, { "b", 2 }, { "c", 3 } });
    auto riter = obj.as_object().rbegin();
    ensure_eq(riter->first, "c");
    ++riter;
    ensure_eq(riter->first, "b");
    ++riter;
    ensure_eq(riter->first, "a");
    ++riter;
    ensure(riter == obj.as_object().rend());
}

TEST(object_compare)
{
    using namespace jsonv;
    
    value obj = object();
    value i = 5;
    
    // really just a test to see if this compiles:
    ensure(obj != i);
}

TEST(object_erase_key)
{
    jsonv::value obj = jsonv::object({ { "foo", 5 }, { "bar", "wat" } });
    ensure_eq(obj.size(), 2);
    ensure_eq(obj.count("bar"), 1);
    ensure_eq(obj.count("foo"), 1);
    ensure_eq(obj.erase("foo"), 1);
    ensure_eq(obj.count("bar"), 1);
    ensure_eq(obj.count("foo"), 0);
    ensure_eq(obj.erase("foo"), 0);
}

TEST(object_erase_iter)
{
    jsonv::value obj = jsonv::object({ { "foo", 5 }, { "bar", "wat" } });
    ensure_eq(obj.size(), 2);
    ensure_eq(obj.count("bar"), 1);
    ensure_eq(obj.count("foo"), 1);
    auto iter = obj.find("bar");
    ensure_eq(iter->first, "bar");
    iter = obj.erase(iter);
    ensure_eq(obj.count("bar"), 0);
    ensure_eq(obj.count("foo"), 1);
    ensure_eq(obj.erase("bar"), 0);
    ensure_eq(iter->first, "foo");
}

TEST(object_erase_whole)
{
    jsonv::value obj = jsonv::object({ { "foo", 5 }, { "bar", "wat" } });
    ensure_eq(obj.size(), 2);
    ensure_eq(obj.count("bar"), 1);
    ensure_eq(obj.count("foo"), 1);
    auto iter = obj.begin_object();
    ensure_eq(iter->first, "bar");
    iter = obj.erase(iter, obj.end_object());
    ensure_eq(obj.size(), 0);
    ensure_eq(obj.count("bar"), 0);
    ensure_eq(obj.count("foo"), 0);
    ensure_eq(obj.erase("bar"), 0);
    ensure(iter == obj.end_object());
    ensure(iter == obj.begin_object());
}

TEST(object_view)
{
    const jsonv::value obj1 = jsonv::object({ { "foo", 5 }, { "bar", "wat" } });
    jsonv::value obj2 = jsonv::object();
    for (const std::pair<const std::string, const jsonv::value>& entry : obj1.as_object())
        obj2.insert(entry);
    ensure_eq(obj1, obj2);
}

TEST(object_nested_access)
{
    jsonv::value v = jsonv::object({ { "x", 0 } });
    jsonv::value* p = &v;
    int depth = 1;
    for (std::string name : { "a", "b", "c", "d" })
    {
        (*p)[name] = jsonv::object({ { "x", depth } });
        p = &(*p)[name];
        ++depth;
    }
    
    ensure_eq(v["x"],                     0);
    ensure_eq(v["a"]["x"],                1);
    ensure_eq(v["a"]["b"]["x"],           2);
    ensure_eq(v["a"]["b"]["c"]["x"],      3);
    ensure_eq(v["a"]["b"]["c"]["d"]["x"], 4);
}

TEST(object_wide_nested_access)
{
    jsonv::value v = jsonv::object({ { "x", 0 } });
    jsonv::value* p = &v;
    int depth = 1;
    for (std::string name : { "a", "b", "c", "d" })
    {
        (*p)[name] = jsonv::object({ { "x", depth } });
        p = &(*p)[name];
        ++depth;
    }
    
    ensure_eq(v.at(L"x"),                      0);
    ensure_eq(v[L"a"][L"x"],                   1);
    ensure_eq(v[L"a"][L"b"][L"x"],             2);
    ensure_eq(v[L"a"][L"b"][L"c"][L"x"],       3);
    ensure_eq(v[L"a"][L"b"][L"c"][L"d"][L"x"], 4);
}

TEST(owning_object_view)
{
    auto view = jsonv::object({ { "a", 1 }, { "b", 2 } }).as_object();
    auto iter = view.begin();
    ensure_eq("a", iter->first);
    ++iter;
    ensure_eq("b", iter->first);
    ++iter;
    ensure(iter == view.end());
}

// An object constructed with wide strings should be the same as one constructed with narrow ones
TEST(object_wide_keys)
{
    auto wobj = jsonv::object({ { L"a", 1 }, { L"b", 2 } });
    auto nobj = jsonv::object({ {  "a", 1 }, {  "b", 2 } });
    ensure_eq(nobj, wobj);
}

TEST(parse_empty_object)
{
    auto obj = jsonv::parse("{}");
    
    ensure(obj.size() == 0);
}

TEST(parse_keyless_object)
{
    try
    {
        jsonv::parse("{a : 3}", jsonv::parse_options().failure_mode(jsonv::parse_options::on_error::collect_all));
    }
    catch (const jsonv::parse_error& err)
    {
        ensure_eq(jsonv::object({ { "a", 3 } }), err.partial_result());
    }
}

TEST(parse_object_wrong_kind_keys)
{
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({1: "blah")"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({null: "blah")"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({true: "blah")"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({1.3: "blah")"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({["hi"]: "blah")"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({{}: "blah")"));
}

TEST(parse_object_stops)
{
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({"a")"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({"a" )"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({"a":)"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({"a": "blah")"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({"a": "blah",)"));
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({"a": "blah", )"));
}

TEST(parse_object_value_stops)
{
    ensure_throws(jsonv::parse_error, jsonv::parse(R"({"a": "blah)"));
}

TEST(parse_object_duplicate_keys)
{
    std::string source = R"({ "a": 1, "a": 2 })";
    ensure_throws(jsonv::parse_error, jsonv::parse(source));
    ensure_eq(jsonv::object({ { "a", 2 } }),
              jsonv::parse(source, jsonv::parse_options().failure_mode(jsonv::parse_options::on_error::ignore))
             );
}
