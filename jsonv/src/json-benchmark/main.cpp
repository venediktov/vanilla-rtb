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
#include "core.hpp"

#include <jsonv/all.hpp>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>

#include <boost/lexical_cast.hpp>

using namespace jsonv;

struct generated_json_settings
{
    std::uniform_int_distribution<int> kind_distribution{0, 6};
    
    template <typename TRng>
    jsonv::kind kind(TRng& rng, std::size_t current_depth)
    {
        if (current_depth < 2)
            return jsonv::kind::object;
        
        auto k = static_cast<jsonv::kind>(kind_distribution(rng));
        if (current_depth > 5 && (k == jsonv::kind::array || k == jsonv::kind::object))
            return kind(rng, current_depth);
        else
            return k;
    }
    
    std::uniform_int_distribution<std::size_t> array_length_distribution{0, 100};
    template <typename TRng>
    std::size_t array_length(TRng& rng)
    {
        return array_length_distribution(rng);
    }
    
    std::uniform_int_distribution<std::size_t> object_size_distribution{5, 25};
    template <typename TRng>
    std::size_t object_size(TRng& rng)
    {
        return object_size_distribution(rng);
    }
    
    std::uniform_int_distribution<std::int64_t> string_length_distribution{0, 100};
    template <typename TRng>
    std::size_t string_length(TRng& rng)
    {
        return string_length_distribution(rng);
    }
    
    
    std::normal_distribution<> decimal_distribution{1.0, 40.0};
    template <typename TRng>
    double decimal(TRng& rng)
    {
        return decimal_distribution(rng);
    }
    
    std::uniform_int_distribution<std::int64_t> integer_distribution{~0};
    template <typename TRng>
    std::int64_t integer(TRng& rng)
    {
        return integer_distribution(rng);
    }
};

template <typename TRng, typename TSettings>
jsonv::value generate_json(TRng& rng, TSettings& settings, std::size_t current_depth = 0)
{
    switch (settings.kind(rng, current_depth))
    {
        case kind::array:
        {
            value out = array();
            std::size_t len = settings.array_length(rng);
            for (std::size_t idx = 0; idx < len; ++idx)
                out.push_back(generate_json(rng, settings, current_depth + 1));
            return out;
        }
        case kind::boolean:
            return !(rng() & 1);
        case kind::decimal:
            return settings.decimal(rng);
        case kind::integer:
            return settings.integer(rng);
        case kind::string:
            // TODO: Improve this
            return std::string(settings.string_length(rng), 'a');
        case kind::object:
        {
            value out = object();
            std::size_t len = settings.object_size(rng);
            for (std::size_t idx = 1; idx <= len; ++idx)
            {
                // TODO: Improve
                std::string key(idx, 'a');
                value val = generate_json(rng, settings, current_depth + 1);
                out.insert({ std::move(key), std::move(val) });
            }
            return out;
        }
        case kind::null:
        default:
            return null;
    }
}

class stopwatch
{
public:
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using duration   = std::chrono::nanoseconds;
    
    struct ticker
    {
        stopwatch* owner;
        time_point start_time;
        
        ticker(stopwatch* o) :
                owner(o),
                start_time(clock::now())
        { }
        
        ticker(ticker&& src) :
                owner(src.owner),
                start_time(src.start_time)
        {
            src.owner = nullptr;
        }
        
        ~ticker()
        {
            if (owner)
                owner->add_time(clock::now() - start_time);
        }
    };
    
public:
    stopwatch() :
            tick_count(0),
            total_time(duration(0))
    { }
    
    ticker start()
    {
        return ticker(this);
    }
    
    void add_time(duration dur)
    {
        ++tick_count;
        total_time += dur;
    }
    
public:
    std::size_t tick_count;
    duration    total_time;
};

static std::string get_encoded_json()
{
    std::ifstream in("temp.json");
    if (in.good())
    {
        std::stringstream buff;
        buff << in.rdbuf();
        return buff.str();
    }
    
    std::ostringstream encoded_stream;
    ostream_pretty_encoder out(encoded_stream);
    std::mt19937_64 rng{std::random_device()()};
    generated_json_settings settings;
    value val = generate_json(rng, settings);
    out.encode(val);
    std::string encoded = encoded_stream.str();
    
    // save the string to a file in case we want to re-use it
    std::ofstream file("temp.json", std::ofstream::out | std::ofstream::trunc);
    file << encoded;
    return encoded;
}

int main(int argc, char** argv)
{
    using namespace json_benchmark;
    
    std::string filter;
    if (argc >= 2)
        filter = argv[1];
    
    int loop_count = 10;
    if (argc >= 3)
        loop_count = boost::lexical_cast<int>(argv[2]);
    
    std::string encoded = get_encoded_json();
    
    for (const benchmark_suite* suite : benchmark_suite::all())
    {
        if (!filter.empty() && filter != suite->name())
            continue;
        
        std::cout << suite->name() << "..." << std::endl;
        stopwatch watch;
        for (int idx = 1; idx <= loop_count; ++idx)
        {
            std::cout << "  " << idx << '/' << loop_count << std::endl;
            auto ticker = watch.start();
            suite->parse_test(encoded);
        }
        
        auto average = std::chrono::duration_cast<std::chrono::microseconds>(watch.total_time) / watch.tick_count;
        std::cout << suite->name() << '\t' << average.count() << "us" << std::endl;
    }
}
