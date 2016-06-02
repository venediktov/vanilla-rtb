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
#ifndef __JSON_BENCHMARK_CORE_HPP_INCLUDED__
#define __JSON_BENCHMARK_CORE_HPP_INCLUDED__

#include <deque>
#include <memory>
#include <string>

namespace json_benchmark
{

class benchmark_suite
{
public:
    using value_ptr = std::shared_ptr<void>;
    
    using suite_list = std::deque<benchmark_suite*>;
    
public:
    static const suite_list& all();
    
    explicit benchmark_suite(std::string name);
    
    virtual ~benchmark_suite() noexcept;
    
    benchmark_suite(const benchmark_suite&) = delete;
    benchmark_suite& operator=(const benchmark_suite&) = delete;
    
    const std::string& name() const { return _name; }
    
    virtual void parse_test(const std::string& source) const = 0;
    
    virtual value_ptr create_value(const std::string& source) const = 0;
    
private:
    std::string _name;
};

template <typename TValue>
class typed_benchmark_suite :
        public benchmark_suite
{
public:
    using value_type = TValue;
    
    using benchmark_suite::benchmark_suite;
    
    virtual void parse_test(const std::string& source) const override
    {
        value_type x = parse(source);
        static_cast<void>(x);
    }
    
    virtual value_ptr create_value(const std::string& source) const override
    {
        return std::make_shared<value_type>(parse(source));
    }
    
protected:
    virtual value_type parse(const std::string& source) const = 0;
};

}

#endif/*__JSON_BENCHMARK_CORE_HPP_INCLUDED__*/
