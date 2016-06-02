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
#include "filesystem_util.hpp"

#include <jsonv/parse.hpp>
#include <jsonv/util.hpp>
#include <jsonv/value.hpp>

#include <fstream>

namespace jsonv_test
{

using namespace jsonv;

class json_diff_test :
        public unit_test
{
public:
    json_diff_test(std::string path) :
            unit_test(std::string("diff_test/") + filename(path)),
            path(std::move(path))
    { }
    
    virtual void run_impl() override
    {
        value whole;
        {
            std::ifstream in(path.c_str());
            whole = parse(in);
        }
        
        diff_result result = diff(whole.at_path(".input.left"), whole.at_path(".input.right"));
        diff_result expected;
        expected.same  = whole.at_path(".result.same");
        expected.right = whole.at_path(".result.right");
        expected.left  = whole.at_path(".result.left");
        
        ensure_eq(expected.same,  result.same);
        ensure_eq(expected.left,  result.left);
        ensure_eq(expected.right, result.right);
    }
    
private:
    std::string path;
};

class json_diff_test_initializer
{
public:
    explicit json_diff_test_initializer(const std::string& rootpath)
    {
        recursive_directory_for_each(rootpath, ".json", [this] (const std::string& p)
        {
            _tests.emplace_back(new json_diff_test(p));
        });
    }
    
private:
    std::deque<std::unique_ptr<unit_test>> _tests;
} json_diff_test_initializer_instance(test_path("diffs"));

}
