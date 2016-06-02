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
#include <jsonv/util.hpp>
#include <jsonv/algorithm.hpp>
#include <jsonv/string_view.hpp>
#include <jsonv/coerce.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace jsonv
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Merging                                                                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

merge_rules::~merge_rules() noexcept = default;

dynamic_merge_rules::dynamic_merge_rules(same_key_function same_key, type_conflict_function type_conflict) :
        same_key(std::move(same_key)),
        type_conflict(std::move(type_conflict))
{ }

dynamic_merge_rules::~dynamic_merge_rules() noexcept = default;

value dynamic_merge_rules::resolve_same_key(path&& current_path, value&& a, value&& b) const
{
    return same_key(std::move(current_path), std::move(a), std::move(b));
}

value dynamic_merge_rules::resolve_type_conflict(path&& current_path, value&& a, value&& b) const
{
    return same_key(std::move(current_path), std::move(a), std::move(b));
}

value throwing_merge_rules::resolve_type_conflict(path&& current_path, value&& a, value&& b) const
{
    throw kind_error(std::string("Found different types at path `") + to_string(current_path) + "': "
                     + to_string(a.kind()) + " and " + to_string(b.kind()));
}

value throwing_merge_rules::resolve_same_key(path&& current_path, value&&, value&&) const
{
    throw std::logic_error(std::string("Cannot merge duplicate key at \"") + to_string(current_path) + "\"");
}

value recursive_merge_rules::resolve_same_key(path&& current_path, value&& a, value&& b) const
{
    return merge_explicit(*this, std::move(current_path), std::move(a), std::move(b));
}

value recursive_merge_rules::resolve_type_conflict(path&&, value&& a, value&& b) const
{
    return coerce_merge(std::move(a), std::move(b));
}

value merge_explicit(const merge_rules& rules,
                     path               current_path,
                     value              a,
                     value              b
                    )
{
    if (  a.kind() != b.kind()
       && !(   (a.kind() == kind::integer && b.kind() == kind::decimal)
            || (a.kind() == kind::decimal && b.kind() == kind::integer)
           )
       )
        return rules.resolve_type_conflict(std::move(current_path), std::move(a), std::move(b));
    
    switch (a.kind())
    {
        case kind::object:
        {
            value out = object();
            for (value::object_iterator iter = a.begin_object(); iter != a.end_object(); iter = a.erase(iter))
            {
                auto iter_b = b.find(iter->first);
                if (iter_b == b.end_object())
                {
                    out.insert({ iter->first, std::move(iter->second) });
                }
                else
                {
                    out.insert({ iter->first,
                                 rules.resolve_same_key(current_path + iter->first,
                                                        std::move(iter->second),
                                                        std::move(iter_b->second)
                                                       )
                               }
                              );
                    b.erase(iter_b);
                }
            }
            
            for (value::object_iterator iter = b.begin_object(); iter != b.end_object(); ++iter)
            {
                out.insert({ iter->first, std::move(iter->second) });
            }
            
            return out;
        }
        case kind::array:
            a.insert(a.end_array(), std::make_move_iterator(b.begin_array()), std::make_move_iterator(b.end_array()));
            return a;
        case kind::boolean:
            return a.as_boolean() || b.as_boolean();
        case kind::integer:
            if (b.kind() == kind::integer)
                return a.as_integer() + b.as_integer();
            // fall through to decimal handler if b is a decimal
        case kind::decimal:
            return a.as_decimal() + b.as_decimal();
        case kind::null:
            return a;
        case kind::string:
            return a.as_string() + b.as_string();
        default:
            throw kind_error(std::string("Invalid kind ") + to_string(a.kind()));
    }
}

value merge_explicit(const merge_rules&, const path&, value a)
{
    return a;
}

value merge_explicit(const merge_rules&, const path&)
{
    return object();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Diff                                                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

diff_result diff(value left, value right)
{
    diff_result result;
    if (left == right)
    {
        result.same = std::move(left);
    }
    else if (left.kind() != right.kind())
    {
        result.left  = std::move(left);
        result.right = std::move(right);
    }
    else switch (left.kind())
    {
    case kind::boolean:
    case kind::decimal:
    case kind::integer:
    case kind::null:
    case kind::string:
        result.left = std::move(left);
        result.right = std::move(right);
        break;
    case kind::array:
        result.same = array();
        result.left = array();
        result.right = array();
        for (value::size_type idx = 0; idx < std::min(left.size(), right.size()); ++idx)
        {
            diff_result subresult = diff(std::move(left.at(idx)), std::move(right.at(idx)));
            result.same.push_back(std::move(subresult.same));
            result.left.push_back(std::move(subresult.left));
            result.right.push_back(std::move(subresult.right));
        }
        
        if (left.size() > right.size())
            result.left.insert(result.left.end_array(),
                               std::make_move_iterator(left.begin_array() + right.size()),
                               std::make_move_iterator(left.end_array())
                              );
        else if (left.size() < right.size())
            result.right.insert(result.right.end_array(),
                                std::make_move_iterator(right.begin_array() + left.size()),
                                std::make_move_iterator(right.end_array())
                               );
        break;
    case kind::object:
        result.same = object();
        result.left = object();
        result.right = object();
        for (value::object_iterator liter = left.begin_object();
             liter != left.end_object();
             liter = left.erase(liter)
            )
        {
            auto riter = right.find(liter->first);
            if (riter == right.end_object())
            {
                result.left.insert({ liter->first, std::move(liter->second) });
            }
            else if (liter->second == riter->second)
            {
                result.same[liter->first] = std::move(liter->second);
                right.erase(riter);
            }
            else
            {
                diff_result subresult = diff(std::move(liter->second), std::move(riter->second));
                result.left[liter->first] = std::move(subresult.left);
                result.right[liter->first] = std::move(subresult.right);
                right.erase(riter);
            }
        }
        
        for (value::object_iterator riter = right.begin_object();
             riter != right.end_object();
             riter = right.erase(riter)
            )
        {
            result.right.insert({ riter->first, std::move(riter->second) });
        }
        break;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Validation                                                                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string validation_error_whatstring(validation_error::code code, const path& p, const value& elem)
{
    std::ostringstream ss;
    ss << "Validation error: Got " << code << " at path " << p << ": " << elem;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const validation_error::code& code)
{
    switch (code)
    {
    case validation_error::code::non_finite_number: return os << "non-finite number";
    default:                                        return os << "validation_error::code(" << static_cast<int>(code) << ")";
    }
}

validation_error::validation_error(code code_, jsonv::path path_, jsonv::value value_) :
        runtime_error(validation_error_whatstring(code_, path_, value_)),
        _code(code_),
        _path(std::move(path_)),
        _value(std::move(value_))
{ }

validation_error::~validation_error() noexcept = default;

validation_error::code validation_error::error_code() const
{
    return _code;
}

const path& validation_error::path() const
{
    return _path;
}

const value& validation_error::value() const
{
    return _value;
}

void validate(const value& val)
{
    traverse(val,
             [] (const path& p, const value& elem)
             {
                 if (elem.kind() == kind::decimal)
                 {
                     if (!std::isfinite(elem.as_decimal()))
                         throw validation_error(validation_error::code::non_finite_number, p, elem);
                 }
             }
            );
}

}
