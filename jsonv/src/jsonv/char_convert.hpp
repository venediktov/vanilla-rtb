/** \file
 *  
 *  Copyright (c) 2012 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_CHAR_CONVERT_HPP_INCLUDED__
#define __JSONV_CHAR_CONVERT_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/parse.hpp>
#include <jsonv/string_view.hpp>

#include <string>
#include <stdexcept>

namespace jsonv
{
namespace detail
{

class decode_error :
        public std::runtime_error
{
public:
    typedef std::string::size_type size_type;
public:
    decode_error(size_type offset, const std::string& message);
    
    virtual ~decode_error() noexcept;
    
    inline size_type offset() const
    {
        return _offset;
    }
    
private:
    size_type _offset;
};

/** Encodes C++ string \a source into a fully-escaped JSON string into \a stream ready for sending over the wire.
**/
std::ostream& string_encode(std::ostream& stream, string_view source, bool ensure_ascii = true);

/** A function that decodes an over the wire character sequence \c source into a C++ string. **/
typedef std::string (*string_decode_fn)(string_view source);

/** Get a string decoding function for the given output \a encoding. **/
string_decode_fn get_string_decoder(parse_options::encoding encoding);

/** Convert the UTF-8 encoded \a source into a UTF-16 encoded \c std::wstring. **/
std::wstring convert_to_wide(string_view source);

/** Convert the UTF-16 encoded \a source into a UTF-8 encoded \c std::string. **/
std::string convert_to_narrow(const std::wstring& source);
std::string convert_to_narrow(const wchar_t*      source);

}
}

#endif/*__JSONV_CHAR_CONVERT_HPP_INCLUDED__*/
