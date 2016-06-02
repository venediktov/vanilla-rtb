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

#include <utility>

namespace json_benchmark
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// benchmark_suite                                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static benchmark_suite::suite_list& benchmark_suite_all_ref()
{
    static benchmark_suite::suite_list instance;
    return instance;
}

const benchmark_suite::suite_list& benchmark_suite::all()
{
    return benchmark_suite_all_ref();
}

benchmark_suite::benchmark_suite(std::string name) :
        _name(std::move(name))
{
    benchmark_suite_all_ref().push_back(this);
}

benchmark_suite::~benchmark_suite() noexcept
{ }

}
