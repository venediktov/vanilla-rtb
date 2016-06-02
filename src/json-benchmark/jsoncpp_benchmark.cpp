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

#include <json/value.h>
#include <json/reader.h>

namespace json_benchmark
{

class jsoncpp_benchmark_suite :
        public typed_benchmark_suite<Json::Value>
{
public:
    jsoncpp_benchmark_suite() :
            typed_benchmark_suite<Json::Value>("JsonCpp")
    { }
    
protected:
    virtual Json::Value parse(const std::string& source) const
    {
        Json::Reader reader;
        Json::Value out;
        if (!reader.parse(source, out))
            throw std::runtime_error("Failed to parse");
        return out;
    }
    
} jsoncpp_benchmark_suite_instance;

}
