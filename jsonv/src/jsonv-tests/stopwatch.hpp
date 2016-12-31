/** \file
 *  Simple timing.
 *
 *  Copyright (c) 2016 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_TESTS_STOPWATCH_HPP_INCLUDED__
#define __JSONV_TESTS_STOPWATCH_HPP_INCLUDED__

#include <jsonv/detail/scope_exit.hpp>

#include <chrono>
#include <iosfwd>

namespace jsonv_test
{

class stopwatch
{
public:
    using clock      = std::chrono::steady_clock;
    using time_point = typename clock::time_point;
    using duration   = typename clock::duration;
    using size_type  = std::size_t;

    struct values
    {
        size_type count;
        duration  sum;
        duration  mean;
    };

public:
    stopwatch() :
            _count(0),
            _sum(0)
    { }

    time_point now() const
    {
        return clock::now();
    }

    void note(duration value)
    {
        _sum += value;
        ++_count;
    }

    void note_now(const time_point& start_time)
    {
        return note(clock::now() - start_time);
    }

    values get() const
    {
        values out;
        out.count = _count;
        out.sum   = _sum;
        out.mean  = _count > 0 ? duration(_sum.count() / _count) : duration(0);
        return out;
    }

private:
    size_type _count;
    duration  _sum;
};

std::ostream& operator<<(std::ostream& os, const stopwatch::values& x);

#define JSONV_TEST_TIME_CONCAT_IMPL(a, b) a ## b
#define JSONV_TEST_TIME_CONCAT(a, b) JSONV_TEST_TIME_CONCAT_IMPL(a, b)
#define JSONV_TEST_TIME_VAR(name) JSONV_TEST_TIME_CONCAT(name, __LINE__)

#define JSONV_TEST_TIME(stopwatch_name)                                                                                \
    auto JSONV_TEST_TIME_VAR(start_time_) = (stopwatch_name).now();                                                    \
    auto JSONV_TEST_TIME_VAR(scope_exit_) = ::jsonv::detail::on_scope_exit([&] ()                                      \
                                            {                                                                          \
                                                (stopwatch_name).note_now(JSONV_TEST_TIME_VAR(start_time_));           \
                                            }                                                                          \
                                           )

}

#endif/*__JSONV_TESTS_STOPWATCH_HPP_INCLUDED__*/
