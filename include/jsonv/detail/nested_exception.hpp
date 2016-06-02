/** \file jsonv/detail/nested_exception.hpp
*
*  Copyright (c) 2015 by Travis Gockel. All rights reserved.
*
*  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
*  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
*  version.
*
*  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_NESTED_EXCEPTION_HPP_INCLUDED__
#define __JSONV_DETAIL_NESTED_EXCEPTION_HPP_INCLUDED__

#include <jsonv/config.hpp>

#include <exception>

namespace jsonv
{

#ifdef _MSC_VER

/** Component for exception classes that captures the currently handled exception as nested. This class is only used in
 *  Microsoft Visual C++ -- as of MSVC 14 CTP 5, there is no definition for std::nested_exception.
**/
class JSONV_PUBLIC nested_exception
{
public:
    nested_exception() noexcept :
            _nested(std::current_exception())
    { }

    nested_exception(const nested_exception& src) noexcept :
            _nested(src._nested)
    { }

    nested_exception& operator=(const nested_exception& src) noexcept
    {
        _nested = src._nested;
        return *this;
    }

    virtual ~nested_exception() noexcept = default;

    __declspec(noreturn) void rethrow_nested() const
    {
        std::rethrow_exception(_nested);
    }

    std::exception_ptr nested_ptr() const noexcept
    {
        return _nested;
    }

private:
    std::exception_ptr _nested;
};

#else

using nested_exception = std::nested_exception;

#endif

}

#endif/*__JSONV_DETAIL_NESTED_EXCEPTION_HPP_INCLUDED__*/
