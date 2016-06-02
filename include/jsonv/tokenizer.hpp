/** \file jsonv/tokenizer.hpp
 *  A stream-based tokenizer meant to help with creating custom parsers. If you are happy with the JSON Voorhees AST
 *  (\c value and friends), it is probably easier to use the functions in \c jsonv/parse.hpp.
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_TOKENIZER_INCLUDED__
#define __JSONV_TOKENIZER_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/string_view.hpp>

#include <iosfwd>
#include <vector>

namespace jsonv
{

/** The kind of token that was encountered in a \c tokenizer. The tokenizer will is parsing this information anyway, so
 *  it is easy to expose.
**/
enum class token_kind : unsigned int
{
    /** Unknown value...either uninitialized or a parse error. **/
    unknown                 = 0x00000,
    /** The beginning of an array: \c [. **/
    array_begin             = 0x00001,
    /** The end of an array: \c ]. **/
    array_end               = 0x00002,
    /** A boolean: \c true or \c false. **/
    boolean                 = 0x00004,
    /** The literal \c null. **/
    null                    = 0x00008,
    /** A number -- in either integer or decimal type. **/
    number                  = 0x00010,
    /** A separator was encountered: \c ,. **/
    separator               = 0x00020,
    /** A string was encountered. It could be the key of an object, but it is not the responsibility of the \c tokenizer
     *  to track this.
    **/
    string                  = 0x00040,
    /** The beginning of an object: \c {. **/
    object_begin            = 0x00080,
    /** The delimiter between an object key and value: \c :. **/
    object_key_delimiter    = 0x00100,
    /** The end of an object: \c }. **/
    object_end              = 0x00200,
    /** The whitespace in between things. **/
    whitespace              = 0x00400,
    /** A JSON comment block. **/
    comment                 = 0x00800,
    /** Indicates that a parse error happened. **/
    parse_error_indicator   = 0x10000,
};

/** Combine multiple flag values. **/
constexpr token_kind operator|(token_kind a, token_kind b)
{
    return token_kind(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

/** Filter out flag values. **/
constexpr token_kind operator&(token_kind a, token_kind b)
{
    return token_kind(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
}

/** Invert flag values. **/
constexpr token_kind operator~(token_kind a)
{
    return token_kind(~static_cast<unsigned int>(a));
}

/** Output the given \c token_kind to the \c std::ostream. **/
JSONV_PUBLIC std::ostream& operator<<(std::ostream&, const token_kind&);

/** Convert the given \c token_kind to an \c std::string. **/
JSONV_PUBLIC std::string to_string(const token_kind&);

/** Splits input into tokens, allowing traversal of JSON without verification. This is the basis for JSON parsers.
 *  
 *  An important thing to remember is a \c tokenizer does not perform any real validation of any kind beyond emitting
 *  a \c token_kind::unknown when it encounters complete garbage. What does this mean? Given the input string:
 *  
 *  \code
 *  [{]{{{{{{}}{{]]]]][][]]][[[[]]]"fdsadf"]]]}}}}}}}}]]]
 *  \endcode
 *  
 *  A \c tokenizer will emit \c token_kind::array_begin, \c token_kind::object_begin, \c token_kind::array_end and so
 *  on, even though it is illegal JSON. It is up to a higher-level construct to detect such failures.
**/
class JSONV_PUBLIC tokenizer
{
public:
    using size_type = std::vector<char>::size_type;
    
    /** Get the minimum size of the internal buffer.
     *  
     *  \see set_min_buffer_size
    **/
    static size_type min_buffer_size();
    
    /** Set the minimum size of the internal buffer of the tokenizer. If you expect to be parsing large JSON strings and
     *  have the memory, performance will be improved by increasing this value.
     *  
     *  \see buffer_reserve
    **/
    static void set_min_buffer_size(size_type sz);
    
    /** A representation of what this tokenizer has. **/
    struct token
    {
        string_view text;
        token_kind kind;
        
        operator std::pair<string_view, token_kind>()
        {
            return { text, kind };
        }
    };
    
public:
    /** Construct a tokenizer to read the given \a input. **/
    explicit tokenizer(std::istream& input);
    
    ~tokenizer() noexcept;
    
    /** Get the input this instance is reading from. **/
    const std::istream& input() const;
    
    /** Attempt to go to the next token in the input stream. The contents of \c current will be cleared.
     * 
     *  \returns \c true if another token was obtained; \c false if we reached EOF or an I/O failure. Check \c input to
     *           see which.
    **/
    bool next();
    
    /** Get the current token and its associated \c token_kind.
     *  
     *  \returns The current token.
     *  \throws std::logic_error if \c next has not been called or if it returned \c false.
    **/
    const token& current() const;
    
    /** Reserve a certain amount of space in the buffer. This will have an effect of performance. No matter what the
     *  value of \a sz is, this will never allow the buffer to shrink below \c min_buffer_size.
    **/
    void buffer_reserve(size_type sz);
    
private:
    bool read_input(bool grow_buffer);
    
private:
    std::istream&     _input;
    std::vector<char> _buffer;
    token             _current;      //!< The current token -- the text always resides within _buffer
};

}

#endif/*__JSONV_TOKENIZER_INCLUDED__*/
