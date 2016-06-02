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
#include <jsonv/detail/token_patterns.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>

#include JSONV_REGEX_INCLUDE

namespace jsonv
{
namespace detail
{

namespace regex_ns = JSONV_REGEX_NAMESPACE;

class re_values
{
public:
    static const regex_ns::regex& number()
    {
        return instance().re_number;
    }
    
    /** Like \c number, but will successfully match an EOF-ed value. **/
    static const regex_ns::regex& number_trunc()
    {
        return instance().re_number_trunc;
    }

    static const regex_ns::regex& simplestring()
    {
        return instance().re_simplestring;
    }

private:
    static const re_values& instance()
    {
        static re_values x;
        return x;
    }

    re_values() :
            syntax_options(regex_ns::regex_constants::ECMAScript | regex_ns::regex_constants::optimize),
            re_number(      R"(^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+(\.[0-9]+)?)?)", syntax_options),
            re_number_trunc(R"(^-?[0-9]*(\.[0-9]*)?([eE][+-]?[0-9]*(\.[0-9]*)?)?)", syntax_options),
            re_simplestring(R"(^[a-zA-Z_$][a-zA-Z0-9_$]*)",                         syntax_options)
    { }

private:
    const regex_ns::regex_constants::syntax_option_type syntax_options;
    const regex_ns::regex re_number;
    const regex_ns::regex re_number_trunc;
    const regex_ns::regex re_simplestring;
};

template <std::ptrdiff_t N>
static match_result match_literal(const char* begin, const char* end, const char (& literal)[N], std::size_t& length)
{
    for (length = 0; length < (N-1); ++length)
    {
        if (begin + length == end)
            return match_result::incomplete_eof;
        else if (begin[length] != literal[length])
            return match_result::unmatched;
    }
    return match_result::complete;
}

static match_result match_true(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    kind = token_kind::boolean;
    return match_literal(begin, end, "true", length);
}

static match_result match_false(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    kind = token_kind::boolean;
    return match_literal(begin, end, "false", length);
}

static match_result match_null(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    kind = token_kind::null;
    return match_literal(begin, end, "null", length);
}

static match_result match_pattern(const char*            begin,
                                  const char*            end,
                                  const regex_ns::regex& pattern,
                                  std::size_t&           length
                                 )
{
    regex_ns::cmatch match;
    if (regex_ns::regex_search(begin, end, match, pattern))
    {
        length = match.length(0);
        return begin + length == end ? match_result::complete_eof : match_result::complete;
    }
    else
    {
        length = 1;
        return match_result::unmatched;
    }
}

static match_result match_number(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    kind = token_kind::number;
    regex_ns::cmatch match;
    if (regex_ns::regex_search(begin, end, match, re_values::number()))
    {
        length = match.length(0);
        if (begin + length == end)
            return match_result::complete_eof;
        else switch (begin[length])
        {
        case '.':
        case '-':
        case '+':
        case 'e':
        case 'E':
            return match_result::incomplete_eof;
        default:
            return match_result::complete;
        }
    }
    else
    {
        length = 1;
        return regex_ns::regex_search(begin, end, match, re_values::number_trunc())
               ? match_result::incomplete_eof
               : match_result::unmatched;
    }
    
}

static match_result match_string(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    assert(*begin == '\"');
    
    kind = token_kind::string;
    length = 1;
    
    while (true)
    {
        if (begin + length == end)
            return match_result::incomplete_eof;
        
        if (begin[length] == '\"')
        {
            ++length;
            return match_result::complete;
        }
        else if (begin[length] == '\\')
        {
            if (begin + length + 1 == end)
                return match_result::incomplete_eof;
            else
                length += 2;
        }
        else
        {
            ++length;
        }
    }
}

static match_result match_whitespace(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    kind = token_kind::whitespace;
    for (length = 0; begin != end; ++length, ++begin)
        switch (*begin)
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            continue;
        default:
            return match_result::complete;
        }
    return match_result::complete_eof;
}

static match_result match_comment(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    assert(*begin == '/');
    
    kind = token_kind::comment;
    if (std::distance(begin, end) == 1)
    {
        length = 1;
        return match_result::incomplete_eof;
    }
    else if (begin[1] == '*')
    {
        bool saw_asterisk = false;
        for (length = 2, begin += 2; begin != end; ++length, ++begin)
        {
            if (*begin == '*')
            {
                saw_asterisk = true;
            }
            else if (saw_asterisk && *begin == '/')
            {
                ++length;
                return match_result::complete;
            }
            else
            {
                saw_asterisk = false;
            }
        }
        return match_result::incomplete_eof;
    }
    else
    {
        length = 1;
        return match_result::unmatched;
    }
    
}

match_result attempt_match(const char* begin, const char* end, token_kind& kind, std::size_t& length)
{
    auto result = [&] (match_result r, token_kind kind_, std::size_t length_)
                  {
                      kind = kind_;
                      length = length_;
                      return r;
                  };
    
    if (begin == end)
    {
        return result(match_result::incomplete_eof, token_kind::unknown, 0);
    }
    
    switch (*begin)
    {
    case '[': return result(match_result::complete, token_kind::array_begin,          1);
    case ']': return result(match_result::complete, token_kind::array_end,            1);
    case '{': return result(match_result::complete, token_kind::object_begin,         1);
    case '}': return result(match_result::complete, token_kind::object_end,           1);
    case ':': return result(match_result::complete, token_kind::object_key_delimiter, 1);
    case ',': return result(match_result::complete, token_kind::separator,            1);
    case 't': return match_true( begin, end, kind, length);
    case 'f': return match_false(begin, end, kind, length);
    case 'n': return match_null( begin, end, kind, length);
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return match_number(begin, end, kind, length);
    case '\"':
        return match_string(begin, end, kind, length);
    case ' ':
    case '\t':
    case '\n':
    case '\r':
        return match_whitespace(begin, end, kind, length);
    case '/':
        return match_comment(begin, end, kind, length);
    default:
        return result(match_result::unmatched, token_kind::unknown, 1);
    }
}

path_match_result path_match(string_view input, string_view& match_contents)
{
    if (input.length() < 2)
        return path_match_result::invalid;
    
    match_result result;
    token_kind kind;
    std::size_t length;
    
    switch (input.at(0))
    {
    case '.':
        result = match_pattern(input.data() + 1, input.data() + input.size(), re_values::simplestring(), length);
        if (result == match_result::complete || result == match_result::complete_eof)
        {
            match_contents = input.substr(0, length + 1);
            return path_match_result::simple_object;
        }
        else
        {
            return path_match_result::invalid;
        }
    case '[':
        result = attempt_match(input.data() + 1, input.data() + input.length(), kind, length);
        if (result == match_result::complete || result == match_result::complete_eof)
        {
            if (input.length() == length + 1 || input.at(1 + length) != ']')
                return path_match_result::invalid;
            if (kind != token_kind::string && kind != token_kind::number)
                return path_match_result::invalid;
            
            match_contents = input.substr(0, length + 2);
            return path_match_result::brace;
        }
        else
        {
            return path_match_result::invalid;
        }
    default:
        return path_match_result::invalid;
    }
}

}
}
