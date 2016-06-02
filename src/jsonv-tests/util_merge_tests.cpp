/** \file
 *  Data-driven tests for testing merges.
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
#include "filesystem_util.hpp"

#include <jsonv/value.hpp>
#include <jsonv/parse.hpp>
#include <jsonv/util.hpp>

#include <fstream>

namespace jsonv_test
{

TEST(merge_single)
{
    jsonv::value x = jsonv::merge(jsonv::null);
    ensure_eq(jsonv::null, x);
}

template <typename TMergeRules>
class json_merge_test :
        public unit_test
{
public:
    json_merge_test(const std::string& test_name,
                    jsonv::value       a,
                    jsonv::value       b,
                    jsonv::value       expected
                   ) :
            unit_test(std::string("merge_test/") + test_name),
            a(std::move(a)),
            b(std::move(b)),
            expected(std::move(expected))
    { }
    
    virtual void run_impl() override
    {
        bool expect_failure = expected.kind() == jsonv::kind::string && expected.as_string() == "FAILURE";
        
        try
        {
            TMergeRules rules;
            jsonv::value result = jsonv::merge_explicit(rules, jsonv::path(), a, b);
            ensure(!expect_failure);
            ensure_eq(expected, result);
        }
        catch (...)
        {
            if (!expect_failure)
                throw;
        }
    }
    
private:
    jsonv::value a;
    jsonv::value b;
    jsonv::value expected;
};

class json_merge_test_initializer
{
public:
    explicit json_merge_test_initializer(const std::string& rootpath)
    {
        recursive_directory_for_each(rootpath, ".json", [this] (const std::string& p)
        {
            jsonv::value whole = [&p] { std::ifstream in(p.c_str()); return jsonv::parse(in); }();
            jsonv::value a = whole.at("a");
            jsonv::value b = whole.at("b");
            
            checked_add<jsonv::throwing_merge_rules>(p, "expected", whole, a, b);
            checked_add<jsonv::recursive_merge_rules>(p, "recursive", whole, a, b);
        });
    }
    
private:
    template <typename TMergeRules>
    void checked_add(const std::string&  p,
                     const std::string&  test_name,
                     const jsonv::value& whole,
                     const jsonv::value& a,
                     const jsonv::value& b
                    )
    {
        if (whole.count(test_name))
        {
            jsonv::value expected = whole.at(test_name);
            _tests.emplace_back(new json_merge_test<TMergeRules>
                                    (
                                        filename(p) + "/" + test_name,
                                        a,
                                        b,
                                        expected
                                    )
                                );
        }
    }
    
private:
    std::deque<std::unique_ptr<unit_test>> _tests;
} json_merge_test_initializer_instance(test_path("merges"));

}
