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

#include <jsonv/demangle.hpp>
#include <jsonv/detail/scope_exit.hpp>

#include <iostream>
#include <typeinfo>

namespace jsonv_test
{

TEST(demangle_types)
{
    std::cout << jsonv::demangle(typeid(jsonv::string_view).name());
    jsonv::demangle("_ZN20garbage");
}

TEST(demangle_set_reset)
{
    jsonv::set_demangle_function(nullptr);
    auto cleanup = jsonv::detail::on_scope_exit(jsonv::reset_demangle_function);
    jsonv::demangle(typeid(int).name());
}

}
