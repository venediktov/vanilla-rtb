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
#include <jsonv-tests/test.hpp>

#include <jsonv/detail/token_patterns.hpp>

namespace jsonv_test
{

using namespace jsonv;
using namespace jsonv::detail;

template <std::size_t N>
match_result static_attempt_match(const char (& buffer)[N], token_kind& kind, std::size_t& length)
{
    return attempt_match(buffer, buffer + N - 1, kind, length);
}

TEST(token_attempt_match_literal_true)
{
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match("true", kind, length);
    ensure(result == match_result::complete);
    ensure_eq(token_kind::boolean, kind);
    ensure_eq(4, length);
}

TEST(token_attempt_match_literal_true_incomplete)
{
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match("tru", kind, length);
    ensure(result == match_result::incomplete_eof);
    ensure_eq(token_kind::boolean, kind);
    ensure_eq(3, length);
}

TEST(token_attempt_match_number_integer_incomplete)
{
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match("1234567890", kind, length);
    ensure(result == match_result::complete_eof);
    ensure_eq(token_kind::number, kind);
    ensure_eq(10, length);
}

TEST(token_attempt_match_number_integer_complete)
{
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match("1234567890,", kind, length);
    ensure(result == match_result::complete);
    ensure_eq(token_kind::number, kind);
    ensure_eq(10, length);
}

TEST(token_attempt_match_number_integer_only_minus)
{
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match("-", kind, length);
    ensure(result == match_result::incomplete_eof);
    ensure_eq(token_kind::number, kind);
    ensure_eq(1, length);
}

TEST(token_attempt_match_string_complete)
{
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match("\"1234567890\"", kind, length);
    ensure(result == match_result::complete);
    ensure_eq(token_kind::string, kind);
    ensure_eq(12, length);
}

TEST(token_attempt_match_string_empty_complete)
{
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match("\"\"", kind, length);
    ensure(result == match_result::complete);
    ensure_eq(token_kind::string, kind);
    ensure_eq(2, length);
}

template <std::size_t N>
std::size_t sstrlen(const char (&)[N])
{
    return N-1;
}

TEST(token_attempt_match_string_double_reverse_solidus_before_escaped_quote)
{
    static const char tokens[] = R"("\\\" and keep going")";
    
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match(tokens, kind, length);
    ensure(result == match_result::complete);
    ensure_eq(token_kind::string, kind);
    ensure_eq(sstrlen(tokens), length);
}

TEST(token_attempt_match_comment)
{
    static const char tokens[] = "/**/";
    
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match(tokens, kind, length);
    
    ensure(result == match_result::complete);
    ensure_eq(token_kind::comment, kind);
    ensure_eq(sstrlen(tokens), length);
}

TEST(token_attempt_match_comment_eof)
{
    static const char tokens[] = "/* whatever goes here*";
    
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match(tokens, kind, length);
    
    ensure(result == match_result::incomplete_eof);
    ensure_eq(token_kind::comment, kind);
    ensure_eq(sstrlen(tokens), length);
}

TEST(token_attempt_match_comment_too_short_eof)
{
    static const char tokens[] = "/*/";
    
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match(tokens, kind, length);
    
    ensure(result == match_result::incomplete_eof);
    ensure_eq(token_kind::comment, kind);
    ensure_eq(sstrlen(tokens), length);
}

TEST(token_attempt_match_comment_slash_invalid)
{
    static const char tokens[] = "//";
    
    token_kind kind;
    std::size_t length;
    match_result result = static_attempt_match(tokens, kind, length);
    
    ensure(result == match_result::unmatched);
}

}
