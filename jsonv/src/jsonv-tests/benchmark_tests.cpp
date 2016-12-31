/** \file
 *  
 *  Copyright (c) 2016 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include "test.hpp"
#include "chrono_io.hpp"
#include "filesystem_util.hpp"
#include "stopwatch.hpp"

#include <jsonv/parse.hpp>
#include <jsonv/util.hpp>
#include <jsonv/value.hpp>

#include <fstream>
#include <iostream>

namespace jsonv_test
{

using namespace jsonv;

static const unsigned iterations = JSONV_DEBUG ? 1 : 100;

template <typename THolster, typename FLoader>
static void run_test(FLoader load, const std::string& from)
{
    stopwatch timer;
    for (unsigned cnt = 0; cnt < iterations; ++cnt)
    {
        THolster src_data{load(from)};
        {
            JSONV_TEST_TIME(timer);
            parse(src_data);
        }
    }
    std::cout << timer.get();
}

template <typename THolster>
class benchmark_test :
        public unit_test
{
public:
    using loader = std::string (*)(const std::string&);

public:
    benchmark_test(loader load, std::string source_desc, std::string path) :
            unit_test(std::string("benchmark/") + source_desc + "/" + filename(path)),
            load(load),
            path(std::move(path))
    { }

    virtual void run_impl() override
    {
        run_test<THolster>(load, path);
    }

private:
    loader      load;
    std::string path;
};

class benchmark_test_initializer
{
public:
    explicit benchmark_test_initializer(const std::string& rootpath)
    {
        recursive_directory_for_each(rootpath, ".json", [&, this] (const std::string& path)
        {
            _tests.emplace_back(new benchmark_test<std::ifstream>([] (const std::string& p) { return p; }, "ifstream", path));
            _tests.emplace_back(new benchmark_test<std::string>(load_from_file, "string", path));
        });
    }

    static std::string load_from_file(const std::string& path)
    {
        std::ifstream inputfile(path.c_str());
        std::string out;
        inputfile.seekg(0, std::ios::end);
        out.reserve(inputfile.tellg());
        inputfile.seekg(0, std::ios::beg);

        out.assign(std::istreambuf_iterator<char>(inputfile), std::istreambuf_iterator<char>());
        return out;
    }

private:
    std::deque<std::unique_ptr<unit_test>> _tests;
} benchmark_test_initializer_instance(test_path(""));

}
