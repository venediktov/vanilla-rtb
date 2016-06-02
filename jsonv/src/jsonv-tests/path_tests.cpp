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
#include "filesystem_util.hpp"

#include <jsonv/algorithm.hpp>
#include <jsonv/parse.hpp>
#include <jsonv/path.hpp>
#include <jsonv/value.hpp>

#include <fstream>

namespace jsonv_test
{

using namespace jsonv;

TEST(path_kind_encoding)
{
    ensure_eq(to_string(path_element_kind::array_index), "array_index");
    ensure_eq(to_string(path_element_kind::object_key),  "object_key");
    
    to_string(static_cast<path_element_kind>(~0));
}

TEST(path_element_copy_compares)
{
    path_element elem1("hi");
    path_element elem2(1);
    path_element elem3(1);
    path_element elem4(elem1);
    path_element elem5(elem2);
    
    ensure_eq(elem1, elem1);
    ensure_ne(elem1, elem2);
    ensure_ne(elem1, elem3);
    ensure_eq(elem1, elem4);
    ensure_ne(elem1, elem5);
    
    ensure_ne(elem2, elem1);
    ensure_eq(elem2, elem2);
    ensure_eq(elem2, elem3);
    ensure_ne(elem2, elem4);
    ensure_eq(elem2, elem5);
    
    ensure_ne(elem3, elem1);
    ensure_eq(elem3, elem2);
    ensure_eq(elem3, elem3);
    ensure_ne(elem3, elem4);
    ensure_eq(elem3, elem5);
    
    ensure_eq(elem4, elem1);
    ensure_ne(elem4, elem2);
    ensure_ne(elem4, elem3);
    ensure_eq(elem4, elem4);
    ensure_ne(elem4, elem5);
    
    ensure_ne(elem5, elem1);
    ensure_eq(elem5, elem2);
    ensure_eq(elem5, elem3);
    ensure_ne(elem5, elem4);
    ensure_eq(elem5, elem5);
    
    elem3 = elem1;
    elem1 = elem2;
    
    ensure_eq(elem1, elem1);
    ensure_eq(elem1, elem2);
    ensure_ne(elem1, elem3);
    
    ensure_eq(elem2, elem1);
    ensure_eq(elem2, elem2);
    ensure_ne(elem2, elem3);
    
    ensure_ne(elem3, elem1);
    ensure_ne(elem3, elem2);
    ensure_eq(elem3, elem3);
}

TEST(path_assign)
{
    path p;
    path q = path::create(".b[5][2].crap[\"blah\"]");
    const path golden = path({ "b", 5, 2, "crap", "blah" });
    p = q;
    ensure_eq(p, q);
    ensure_eq(golden, p);
    ensure_eq(golden, q);
    
    q = std::move(p);
    ensure_eq(0, p.size());
    ensure_eq(golden.size(), q.size());
    
    p = std::move(q);
    ensure_eq(golden.size(), p.size());
    ensure_eq(0, q.size());
    
    q = p;
    ensure_eq(p, q);
    ensure_eq(golden, p);
    ensure_eq(golden, q);
    p = q;
    ensure_eq(p, q);
    ensure_eq(golden, p);
    ensure_eq(golden, q);
}

TEST(path_concat_key)
{
    path p({ path_element("a") });
    path q = p + "b";
    ensure_eq(q, path({ path_element("a"), path_element("b") }));
    ensure_eq(to_string(q), ".a.b");
}

TEST(path_traverse)
{
    value tree;
    {
        std::ifstream stream(test_path("paths.json").c_str());
        tree = parse(stream);
    }
    
    traverse(tree,
             [this, &tree] (const path& p, const value& x)
             {
                 ensure_eq(to_string(p), x.as_string());
                 auto q = path::create(x.as_string());
                 ensure_eq(p, q);
                 ensure_eq(x, tree.at_path(p));
                 ensure_eq(x, tree.at_path(q));
             },
             true
            );
}

TEST(path_append_key)
{
    path p;
    p += "a";
    path q({ path_element("a") });
    ensure_eq(p, q);
}

TEST(path_create_simplestring)
{
    path p = path::create(".a.b.c");
    path q({ "a", "b", "c" });
    ensure_eq(p, q);
}

TEST(path_value_construction)
{
    value tree;
    tree.path(".a.b[2]") = "Hello!";
    tree.path(".a[\"b\"][3]") = 3;
    tree.path(".a[\"b\"][1]") = "Yo";
    
    value expected = object({
                             { "a", object({
                                            { "b", array({ null, "Yo", "Hello!", 3 }) }
                                          })
                             }
                           });
    ensure_eq(expected, tree);
}

TEST(value_at_path_out_of_range)
{
    value tree;
    tree.path(".a.b[2]") = "Hello!";
    tree.path(".a[\"b\"][3]") = 3;
    tree.path(".a[\"b\"][1]") = "Yo";
    ensure_eq(1UL, tree.count_path(".a.b[0]"));
    ensure_throws(std::out_of_range, tree.at_path(".does.not.exist"));
    ensure_eq(0UL, tree.count_path(".does.not.exist"));
    const value& tree2 = tree;
    ensure_throws(std::out_of_range, tree2.at_path(".does.not.exist"));
}

TEST(value_path_array_construct)
{
    value arr;
    arr.path(0) = 0;
    arr.path(1) = 1;
    arr.path(2) = 2;
    ensure_eq(arr, array({ 0, 1, 2 }));
    ensure_eq(1UL, arr.count_path(0));
    ensure_eq(1UL, arr.count_path(1));
    ensure_eq(1UL, arr.count_path(2));
    ensure_eq(0UL, arr.count_path(3));
    ensure_eq(0UL, arr.count_path(".a"));
}

TEST(path_element_access)
{
    path p = path::create(".a.b[5]");
    ensure_throws(std::out_of_range, p.at(4));
    ensure_throws(kind_error, p.at(0).index());
    ensure_throws(kind_error, p.at(2).key());
    
    path q = std::move(p);
    p = q;
    ensure_eq(p, q);
}

TEST(path_element_kind_to_string_invalid)
{
    to_string(static_cast<path_element_kind>(~0));
}

TEST(path_parse_invalid)
{
    ensure_throws(std::invalid_argument, path::create(".a#"));
    ensure_throws(std::invalid_argument, path::create("2"));
}

TEST(path_combines)
{
    path goal = path::create(".a.b.c[3][4][5]");
    path a = path::create(".a.b.c");
    path b = path::create("[3][4][5]");
    
    ensure_eq(goal, a + b);
    a += b;
    ensure_eq(goal, a);
}

}
