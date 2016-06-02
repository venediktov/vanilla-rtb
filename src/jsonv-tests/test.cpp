/** \file
 *  
 *  Copyright (c) 2012 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include "test.hpp"

#include <iostream>
#include <typeinfo>

#include <jsonv/demangle.hpp>

namespace jsonv_test
{

unit_test_list_type& get_unit_tests()
{
    static unit_test_list_type instance;
    return instance;
}

unit_test::unit_test(const std::string& name) :
        _name(name)
{
    get_unit_tests().push_back(this);
}
    
bool unit_test::run()
{
    std::cout << "TEST: " << name() << " ...";
    _success = true;
    try
    {
        run_impl();
    }
    catch (const std::exception& ex)
    {
        _success = false;
        _failstring = std::string("Threw exception of type ") + jsonv::demangle(typeid(ex).name()) + ": " + ex.what();
    }
    catch (...)
    {
        _success = false;
        _failstring = "Threw unknown exception";
    }
    if (_success)
        std::cout << " SUCCESS!" << std::endl;
    else
        std::cout << " \x1b[0;31mFAILURE " << _failstring << "\x1b[m" << std::endl;
    return _success;
}

}
