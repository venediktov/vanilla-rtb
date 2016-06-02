/** \file
 *  
 *  Copyright (c) 2012-2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __TEST_JSONV_TEST_HPP_INCLUDED__
#define __TEST_JSONV_TEST_HPP_INCLUDED__

#define ASSERT_ON_TEST_FAILURE 0

#if ASSERT_ON_TEST_FAILURE
#   include <cassert>
#endif

#include <deque>
#include <sstream>
#include <string>

namespace jsonv_test
{

class unit_test;

typedef std::deque<unit_test*> unit_test_list_type;
unit_test_list_type& get_unit_tests();

#if ASSERT_ON_TEST_FAILURE
#   define ensure assert
#else
#   define ensure(cond_)                    \
        do                                  \
        {                                   \
            if (!(cond_))                   \
            {                               \
                this->_success = false;     \
                this->_failstring = #cond_; \
                return;                     \
            }                               \
        } while (0)
#endif

#if ASSERT_ON_TEST_FAILURE
#   define ensure_op(a_, op_, b_) assert((a_) op_ (b_))
#else
#   define ensure_op(a_, op_, b_)                         \
        do                                                \
        {                                                 \
            if (!((a_) op_ (b_)))                         \
            {                                             \
                this->_success = false;                   \
                std::ostringstream ss;                    \
                ss << "!(" << #a_ << " {" << (a_) << "}"; \
                ss << " " << #op_ << " ";                 \
                ss << #b_ << " {" << (b_) << "})";        \
                this->_failstring = ss.str();             \
                return;                                   \
            }                                             \
        } while (0)
#endif

#define ensure_eq(a_, b_) ensure_op(a_, ==, b_)
#define ensure_ne(a_, b_) ensure_op(a_, !=, b_)
#define ensure_lt(a_, b_) ensure_op(a_, < , b_)
#define ensure_le(a_, b_) ensure_op(a_, <=, b_)
#define ensure_gt(a_, b_) ensure_op(a_, > , b_)
#define ensure_ge(a_, b_) ensure_op(a_, >=, b_)

#define ensure_throws(extype_, action_)                              \
    do                                                               \
    {                                                                \
        try                                                          \
        {                                                            \
            action_;                                                 \
            ensure(! #extype_ " was not thrown");                    \
        }                                                            \
        catch (const extype_&)                                       \
        { }                                                          \
    } while (false)                                                  \

class unit_test
{
public:
    explicit unit_test(const std::string& name);
    
    bool run();
    
    const std::string& name() const
    {
        return _name;
    }
    
private:
    virtual void run_impl() = 0;
    
protected:
    std::string _name;
    bool        _success;
    std::string _failstring;
};

#define TEST(name_)                          \
    class name_ ## _test :                   \
            public ::jsonv_test::unit_test   \
    {                                        \
    public:                                  \
        name_ ## _test() :                   \
            ::jsonv_test::unit_test(#name_)  \
        { }                                  \
                                             \
        void run_impl();                     \
    } name_ ## _test_instance;               \
                                             \
    void name_ ## _test::run_impl()

}

#endif/*__TEST_JSONV_TEST_HPP_INCLUDED__*/
