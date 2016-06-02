/** \file
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/tokenizer.hpp>
#include <jsonv/detail/token_patterns.hpp>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <istream>
#include <iterator>
#include <sstream>

namespace jsonv
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// token_kind                                                                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const token_kind& value)
{
    static constexpr token_kind all_valid_tokens = token_kind(0x0fff);
    static constexpr token_kind non_error_tokens = token_kind(0xffff);
    
    switch (value)
    {
    case token_kind::unknown:                return os << "unknown";
    case token_kind::array_begin:            return os << '[';
    case token_kind::array_end:              return os << ']';
    case token_kind::boolean:                return os << "boolean";
    case token_kind::null:                   return os << "null";
    case token_kind::number:                 return os << "number";
    case token_kind::separator:              return os << ',';
    case token_kind::string:                 return os << "string";
    case token_kind::object_begin:           return os << '{';
    case token_kind::object_key_delimiter:   return os << ':';
    case token_kind::object_end:             return os << '}';
    case token_kind::whitespace:             return os << "whitespace";
    case token_kind::comment:                return os << "comment";
    case token_kind::parse_error_indicator:
    default:
        // if the value represents a parse error...
        if ((value & token_kind::parse_error_indicator) == token_kind::parse_error_indicator)
        {
            return os << "parse_error("
                      << (value & all_valid_tokens)
                      << ')';
        }
        // not a parse error
        else
        {
            token_kind post = value & non_error_tokens;
            for (token_kind scan_token = static_cast<token_kind>(1);
                 bool(post);
                 scan_token = token_kind(static_cast<unsigned int>(scan_token) << 1)
                )
            {
                if (bool(scan_token & post))
                {
                    if (bool(scan_token & all_valid_tokens))
                        os << scan_token;
                    else
                        os << std::hex << "0x" << std::setfill('0') << std::setw(4)
                           << static_cast<unsigned int>(scan_token)
                           << std::dec << std::setfill(' ');
                    
                    post = post & ~scan_token;
                    if (bool(post))
                        os << '|';
                }
            }
            return os;
        }
    }
}

std::string to_string(const token_kind& value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// tokenizer                                                                                                          //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static tokenizer::size_type& min_buffer_size_ref()
{
    static tokenizer::size_type instance = 1024 * sizeof(void*);
    return instance;
}

tokenizer::size_type tokenizer::min_buffer_size()
{
    return min_buffer_size_ref();
}

void tokenizer::set_min_buffer_size(tokenizer::size_type sz)
{
    min_buffer_size_ref() = std::max(sz, tokenizer::size_type(1));
}

static std::size_t position_in_buffer(const std::vector<char>& buffer, const string_view& current)
{
    // an invalid current means the buffer is fresh, so we're at the start of it
    if (!current.data())
        return 0;
    
    std::ptrdiff_t pos = current.data() - buffer.data();
    assert(pos >= 0);
    assert(std::size_t(pos) <= buffer.size());
    return std::size_t(pos);
}

tokenizer::tokenizer(std::istream& input) :
        _input(input)
{
    buffer_reserve(min_buffer_size());
}

tokenizer::~tokenizer() noexcept
{
    // getting destroyed -- need to put our buffer back into the istream
    try
    {
        if (_current.text.size() > 0 && _buffer.size() > 0)
        {
            for (std::size_t idx = _buffer.size() - 1; idx != position_in_buffer(_buffer, _current.text); --idx)
                _input.putback(_buffer[idx]);
        }
    }
    catch (...)
    {
        // there's not much we can do here...if the istream doesn't allow us to give things back, we can only hope that
        // the user didn't care.
    }
}

const std::istream& tokenizer::input() const
{
    return _input;
}

const tokenizer::token& tokenizer::current() const
{
    if (_current.text.data())
        return _current;
    else
        throw std::logic_error("Cannot get token -- call next() and make sure it returns true.");
}

bool tokenizer::next()
{
    auto valid = [this] (const string_view& new_current, token_kind new_kind)
                 {
                     _current.text = new_current;
                     _current.kind = new_kind;
                     return true;
                 };
    auto invalid = [this]
                   {
                       _current.text.clear();
                       return false;
                   };
    
    size_type pos;
    bool nth_pass = false;
    _current.text.remove_prefix(_current.text.size());
    while (true)
    {
        if (_buffer.empty() || (position_in_buffer(_buffer, _current.text)) == _buffer.size())
        {
            if (!read_input(nth_pass))
                return invalid();
            nth_pass = true;
        }
        
        pos = position_in_buffer(_buffer, _current.text) + _current.text.size();
        
        token_kind kind;
        size_type match_len;
        auto result = detail::attempt_match(_buffer.data() + pos, _buffer.data() + _buffer.size(),
                                            kind, match_len
                                           );
        if (result == detail::match_result::complete_eof || result == detail::match_result::incomplete_eof)
        {
            // partial match...we need to grow the buffer to see if it keeps going
            if (read_input(true))
                continue;
            else if (result == detail::match_result::incomplete_eof)
                // we couldn't read more data due to EOF...but we have an incomplete match
                kind = kind | token_kind::parse_error_indicator;
        }
        else if (result == detail::match_result::unmatched)
        {
            // unmatched entry -- this token is invalid
            kind = kind | token_kind::parse_error_indicator;
        }
        return valid(string_view(_buffer.data() + pos, match_len), kind);
    }
}

bool tokenizer::read_input(bool grow_buffer)
{
    auto old_buffer_pos = position_in_buffer(_buffer, _current.text);
    if (old_buffer_pos == _buffer.size())
        old_buffer_pos = 0;
    
    char* buffer_write_pos;
    size_type buffer_write_size;
    if (grow_buffer)
    {
        buffer_write_size = min_buffer_size();
        auto offset = _buffer.size();
        _buffer.resize(_buffer.size() + buffer_write_size);
        buffer_write_pos = _buffer.data() + offset;
    }
    else
    {
        buffer_write_size = _buffer.capacity();
        _buffer.resize(buffer_write_size);
        buffer_write_pos = _buffer.data();
    }
    
    _input.read(buffer_write_pos, buffer_write_size);
    auto read_count = _input.gcount();
    if (read_count > 0)
    {
        // check that the resize operation will not change the position of data()
        std::size_t new_size = (buffer_write_pos - _buffer.data()) + read_count;
        assert(_buffer.capacity() >= new_size);
        _buffer.resize(new_size);
        _current.text = string_view(_buffer.data() + old_buffer_pos, _current.text.size());
        return true;
    }
    else
    {
        return false;
    }
}

void tokenizer::buffer_reserve(size_type sz)
{
    _buffer.reserve(std::max(sz, min_buffer_size()));
}

}
