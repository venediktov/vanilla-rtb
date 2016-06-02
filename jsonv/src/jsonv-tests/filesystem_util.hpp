/** \file
 *  Utility functions for interacting with the filesystem.
 *  
 *  Copyright (c) 2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_TESTS_FILESYSTEM_UTIL_HPP_INCLUDED__
#define __JSONV_TESTS_FILESYSTEM_UTIL_HPP_INCLUDED__

#include "test.hpp"

#include <functional>

namespace jsonv_test
{

std::string test_path(const std::string& path);

std::string filename(std::string path);

void recursive_directory_for_each(const std::string&                             root_path,
                                  const std::string&                             extension_filter,
                                  const std::function<void (const std::string&)> action
                                 );

}

#endif/*__JSONV_TESTS_FILESYSTEM_UTIL_HPP_INCLUDED__*/
