/** \file
 *  I/O for the chrono library.
 *
 *  Copyright (c) 2016 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_TESTS_CHRONO_IO_HPP_INCLUDED__
#define __JSONV_TESTS_CHRONO_IO_HPP_INCLUDED__

#include <chrono>
#include <cstdint>
#include <iosfwd>

namespace jsonv_test
{

std::ostream& operator<<(std::ostream&, std::chrono::nanoseconds);

template <typename TRep, typename TPeriod>
std::ostream& operator<<(std::ostream& os, std::chrono::duration<TRep, TPeriod> value)
{
    return os << std::chrono::duration_cast<std::chrono::nanoseconds>(value);
}

}

#endif/*__JSONV_TESTS_CHRONO_IO_HPP_INCLUDED__*/
