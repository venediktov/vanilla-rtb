/** \file
 *  Classes and functions for encoding JSON values to various representations.
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/encode.hpp>
#include <jsonv/value.hpp>

#include "detail.hpp"

#include <cmath>

namespace jsonv
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// encoder                                                                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

encoder::~encoder() noexcept = default;

void encoder::encode(const value& source)
{
    switch (source.kind())
    {
    case kind::array:
        write_array_begin();
        {
            bool first = true;
            for (const value& sub : source.as_array())
            {
                if (first)
                    first = false;
                else
                    write_array_delimiter();
                encode(sub);
            }
        }
        write_array_end();
        break;
    case kind::boolean:
        write_boolean(source.as_boolean());
        break;
    case kind::decimal:
        write_decimal(source.as_decimal());
        break;
    case kind::integer:
        write_integer(source.as_integer());
        break;
    case kind::null:
        write_null();
        break;
    case kind::object:
        write_object_begin();
        {
            bool first = true;
            for (const value::object_value_type& entry : source.as_object())
            {
                if (first)
                    first = false;
                else
                    write_object_delimiter();
                
                write_object_key(entry.first);
                encode(entry.second);
            }
        }
        write_object_end();
        break;
    case kind::string:
        write_string(source.as_string());
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ostream_encoder                                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ostream_encoder::ostream_encoder(std::ostream& output) :
        _output(output),
        _ensure_ascii(true)
{ }

ostream_encoder::~ostream_encoder() noexcept = default;

void ostream_encoder::write_array_begin()
{
    _output << '[';
}

void ostream_encoder::write_array_end()
{
    _output << ']';
}

void ostream_encoder::write_array_delimiter()
{
    _output << ',';
}

void ostream_encoder::write_boolean(bool value)
{
    _output << (value ? "true" : "false");
}

void ostream_encoder::write_decimal(double value)
{
    if (std::isfinite(value))
        _output << value;
    else
        // non-finite values do not have valid JSON representations, so put it as null
        write_null();
}

void ostream_encoder::write_integer(std::int64_t value)
{
    _output << value;
}

void ostream_encoder::write_null()
{
    _output << "null";
}

void ostream_encoder::write_object_begin()
{
    _output << '{';
}

void ostream_encoder::write_object_end()
{
    _output << '}';
}

void ostream_encoder::write_object_delimiter()
{
    _output << ',';
}

void ostream_encoder::write_object_key(string_view key)
{
    write_string(key);
    _output << ':';
}

void ostream_encoder::write_string(string_view value)
{
    stream_escaped_string(_output, value, _ensure_ascii);
}

std::ostream& ostream_encoder::output()
{
    return _output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ostream_pretty_encoder                                                                                             //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ostream_pretty_encoder::ostream_pretty_encoder(std::ostream& output, std::size_t indent_size) :
        ostream_encoder(output),
        _indent(0),
        _indent_size(indent_size),
        _defer_indent(false)
{ }

ostream_pretty_encoder::~ostream_pretty_encoder() noexcept = default;

void ostream_pretty_encoder::write_prefix()
{
    if (_defer_indent)
    {
        write_eol();
        _defer_indent = false;
    }
}

void ostream_pretty_encoder::write_eol()
{
    output() << std::endl;
    for (std::size_t x = 0; x < _indent; ++x)
        output() << ' ';
}

void ostream_pretty_encoder::write_array_begin()
{
    write_prefix();
    ostream_encoder::write_array_begin();
    _indent += _indent_size;
    _defer_indent = true;
}

void ostream_pretty_encoder::write_array_end()
{
    _indent -= _indent_size;
    if (!_defer_indent)
    {
        write_eol();
    }
    _defer_indent = false;
    ostream_encoder::write_array_end();
}

void ostream_pretty_encoder::write_array_delimiter()
{
    write_prefix();
    ostream_encoder::write_array_delimiter();
    write_eol();
}

void ostream_pretty_encoder::write_boolean(bool value)
{
    write_prefix();
    ostream_encoder::write_boolean(value);
}

void ostream_pretty_encoder::write_decimal(double value)
{
    write_prefix();
    ostream_encoder::write_decimal(value);
}

void ostream_pretty_encoder::write_integer(int64_t value)
{
    write_prefix();
    ostream_encoder::write_integer(value);
}

void ostream_pretty_encoder::write_null()
{
    write_prefix();
    ostream_encoder::write_null();
}

void ostream_pretty_encoder::write_object_begin()
{
    write_prefix();
    ostream_encoder::write_object_begin();
    _indent += _indent_size;
    _defer_indent = true;
}

void ostream_pretty_encoder::write_object_end()
{
    _indent -= _indent_size;
    if (!_defer_indent)
    {
        write_eol();
    }
    _defer_indent = false;
    ostream_encoder::write_object_end();
}

void ostream_pretty_encoder::write_object_delimiter()
{
    ostream_encoder::write_object_delimiter();
    _defer_indent = true;
}

void ostream_pretty_encoder::write_object_key(string_view key)
{
    write_prefix();
    ostream_encoder::write_object_key(key);
    output() << ' ';
}

void ostream_pretty_encoder::write_string(string_view value)
{
    write_prefix();
    ostream_encoder::write_string(value);
}

}
