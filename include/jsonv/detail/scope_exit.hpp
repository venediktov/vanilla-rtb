/** \file jsonv/detail/scope_exit.hpp
 *  Definition of the \c on_scope_exit utility.
 *  
 *  Copyright (c) 2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_SCOPE_EXIT_HPP_INCLUDED__
#define __JSONV_DETAIL_SCOPE_EXIT_HPP_INCLUDED__

#include <jsonv/config.hpp>

#include <utility>

namespace jsonv
{
namespace detail
{

template <typename Function>
class scope_exit_invoker
{
public:
    explicit scope_exit_invoker(Function&& func) :
            _func(std::move(func)),
            _responsible(true)
    { }
    
    scope_exit_invoker(scope_exit_invoker&& src) :
            _func(std::move(src._func)),
            _responsible(src._responsible)
    {
        src._responsible = false;
    }
    
    scope_exit_invoker(const scope_exit_invoker&) = delete;
    scope_exit_invoker& operator=(const scope_exit_invoker&) = delete;
    scope_exit_invoker& operator=(scope_exit_invoker&&) = delete;
    
    ~scope_exit_invoker()
    {
        if (_responsible)
            _func();
    }
    
    void release()
    {
        _responsible = false;
    }
    
private:
    Function _func;
    bool     _responsible;
};

template <typename Function>
scope_exit_invoker<Function> on_scope_exit(Function func)
{
    return scope_exit_invoker<Function>(std::move(func));
}

}
}

#endif/*__JSONV_DETAIL_SCOPE_EXIT_HPP_INCLUDED__*/
