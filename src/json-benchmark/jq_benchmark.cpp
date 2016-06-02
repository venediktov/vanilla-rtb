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

extern "C"
{

#include <jv.h>

}

namespace json_benchmark
{

using jq_value = std::shared_ptr<jv>;

class jq_benchmark_suite :
        public typed_benchmark_suite<jq_value>
{
public:
    jq_benchmark_suite() :
            typed_benchmark_suite<jq_value>("jq")
    { }
    
protected:
    virtual jq_value parse(const std::string& source) const
    {
        jq_value x(new jv, [] (jv* p) { jv_free(*p); delete p; });
        *x = jv_parse_sized(source.c_str(), source.size());
        return x;
    }
    
} jq_benchmark_suite_instance;

}
