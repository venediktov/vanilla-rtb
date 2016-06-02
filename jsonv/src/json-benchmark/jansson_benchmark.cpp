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

#include <jansson.h>

namespace json_benchmark
{

using jansson_value = std::shared_ptr<json_t>;

class jansson_benchmark_suite :
        public typed_benchmark_suite<jansson_value>
{
public:
    jansson_benchmark_suite() :
            typed_benchmark_suite<jansson_value>("jansson")
    { }
    
protected:
    virtual jansson_value parse(const std::string& source) const
    {
        json_t* px = json_loads(source.c_str(), 0, nullptr);
        jansson_value x(px, [] (json_t* p) { json_decref(p); });
        return x;
    }
    
} jansson_benchmark_suite_instance;

}
