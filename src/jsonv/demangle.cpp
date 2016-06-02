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
#include <jsonv/demangle.hpp>
#include <jsonv/detail/scope_exit.hpp>

#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#include <cstdlib>

namespace jsonv
{

#ifdef _MSC_VER
static std::string demangle_impl(string_view source)
{
    return std::string(source);
}
#else
static std::string demangle_impl(string_view source)
{
    namespace cxxabi = __cxxabiv1;
    int status;
    char* demangled = cxxabi::__cxa_demangle(source.data(), nullptr, nullptr, &status);
    if (demangled)
    {
        auto cleanup = detail::on_scope_exit([demangled] { std::free(demangled); });
        return std::string(demangled);
    }
    else
    {
        return std::string(source);
    }
}
#endif

static demangle_function& demangle_function_ref()
{
    static demangle_function instance = demangle_impl;
    return instance;
}

void set_demangle_function(demangle_function func)
{
    demangle_function_ref() = std::move(func);
}

void reset_demangle_function()
{
    demangle_function_ref() = demangle_impl;
}

std::string demangle(string_view source)
{
    const demangle_function& func = demangle_function_ref();
    if (func)
        return func(source);
    else
        return std::string(source);
}

}
