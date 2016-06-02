/** \file jsonv/detail/token_patterns.hpp
 *  Pattern matching for JSON tokens.
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_DETAIL_TOKEN_PATTERNS_HPP_INCLUDED__
#define __JSONV_DETAIL_TOKEN_PATTERNS_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/string_view.hpp>
#include <jsonv/tokenizer.hpp>

namespace jsonv
{
namespace detail
{

/** The result of a match. **/
enum class match_result
{
    // A "complete" match -- a whole token was definitely found
    complete,
    // A "complete" match, but it might continue on
    complete_eof,
    // An "incomplete" match -- the pattern looks good so far, but we don't know for sure
    incomplete_eof,
    // Could not find a valid sequence.
    unmatched,
};

/** Attempt to match the given sequence.
 *  
 *  \param begin The beginning of the sequence to attempt to match.
 *  \param end   The end of the sequence to attempt to match.
 *  \param[out] kind The kind of token matched.
 *  \param[out] length The length of the match, if found.
 *  \returns the result of the match.
**/
match_result attempt_match(const char*  begin,
                           const char*  end,
                           token_kind&  kind,
                           std::size_t& length
                          );

enum class path_match_result : char
{
    simple_object = '.',
    brace         = '[',
    invalid       = '\x00',
};

/** Attempt to match a path.
 *  
 *  \param input The input to match
 *  \param[out] match_contents The full contents of a match
**/
path_match_result path_match(string_view  input,
                             string_view& match_contents
                            );

}
}

#endif/*__JSONV_DETAIL_TOKEN_PATTERNS_HPP_INCLUDED__*/
