/** \file jsonv/parse.hpp
 *  
 *  Copyright (c) 2012-2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_PARSE_HPP_INCLUDED__
#define __JSONV_PARSE_HPP_INCLUDED__

#include <jsonv/config.hpp>
#include <jsonv/string_view.hpp>
#include <jsonv/value.hpp>

#include <cstddef>
#include <deque>
#include <stdexcept>

namespace jsonv
{

class tokenizer;

/** An error encountered when parsing.
 *  
 *  \see parse
**/
class JSONV_PUBLIC parse_error :
        public std::runtime_error
{
public:
    typedef std::size_t size_type;
    
    /** Description of a single parsing problem. **/
    struct problem
    {
    public:
        problem(size_type line, size_type column, size_type character, std::string message);
        
        /** The line of input this error was encountered on. A new "line" is determined by carriage return or line feed.
         *  If you are in Windows and line breaks are two characters, the line number of the error will appear to be
         *  twice as high as you would think.
        **/
        size_type line() const
        {
            return _line;
        }
        
        /** The character index on the current line this error was encountered on. **/
        size_type column() const
        {
            return _column;
        }
        
        /** The character index into the entire input this error was encountered on. **/
        size_type character() const
        {
            return _character;
        }
        
        /** A message from the parser which has user-readable details about the encountered problem. **/
        const std::string& message() const
        {
            return _message;
        }
        
    private:
        size_type   _line;
        size_type   _column;
        size_type   _character;
        std::string _message;
    };
    
    typedef std::deque<problem> problem_list;
    
public:
    parse_error(problem_list, value partial_result);
    
    virtual ~parse_error() noexcept;
    
    /** The list of problems which ultimately contributed to this \c parse_error. There will always be at least one
     *  \c problem in this list.
    **/
    const problem_list& problems() const;
    
    /** Get the partial result of parsing. There is no guarantee this value even resembles the input JSON as the input
     *  JSON was malformed.
    **/
    const value& partial_result() const;
    
private:
    problem_list _problems;
    value        _partial_result;
};

/** Get a string representation of a problem. **/
JSONV_PUBLIC std::ostream& operator<<(std::ostream& os, const parse_error::problem& p);

/** Get a string representation of a problem. **/
JSONV_PUBLIC std::string to_string(const parse_error::problem& p);

/** Get a string representation of a \c parse_error. **/
JSONV_PUBLIC std::ostream& operator<<(std::ostream& os, const parse_error& p);

/** Get a string representation of a \c parse_error. **/
JSONV_PUBLIC std::string to_string(const parse_error& p);

/** Configuration for various parsing options. All parse functions should take in a \c parse_options as a paramter and
 *  should respect your settings.
**/
class JSONV_PUBLIC parse_options
{
public:
    using size_type = value::size_type;
    
    /** When a parse error is encountered, what should the parser do? **/
    enum class on_error
    {
        /** Immediately throw a \c parse_error -- do not attempt to construct a partial result. **/
        fail_immediately,
        /** Attempt to continue parsing and constructing a result. **/
        collect_all,
        /** Ignore all errors and pretend to be successful. This is not recommended unless you are 100% certain the
         *  JSON you are attempting to parse is valid. Using this failure mode does not improve parser performance.
        **/
        ignore,
    };
    
    /** The encoding format for strings. **/
    enum class encoding
    {
        /** Use UTF-8 like a sane library should.
         *  
         *  \see http://www.unicode.org/versions/Unicode6.2.0/ch03.pdf#G7404
        **/
        utf8,
        /** Like \c utf8, but check that there are no unprintable characters in the input stream (see \c std::isprint).
         *  To contrast this with \c utf8, this mode will reject things such as the \c tab and \c newline characters,
         *  while this will reject them.
        **/
        utf8_strict,
        /** Use the CESU-8 Compatibility Encoding Scheme for UTF-16? It is generally not recommended unless your
         *  processing environment requires binary collation with UTF-16. If you do not know you need this, you probably
         *  do not.
         *  
         *  \see http://www.unicode.org/reports/tr26/
        **/
        cesu8,
    };
    
    /** When dealing with comma separators, how should extra commas be treated? **/
    enum class commas
    {
        /** Do not allow any extra commas anywhere -- require valid JSON. **/
        strict,
        /** Allow a single trailing comma at the end of an array or object (similar to C++ \c enum definitions). **/
        allow_trailing,
    };
    
    /** How should numbers be dealt with? **/
    enum class numbers
    {
        /** Parse \e all forms of decimal input that we can. To contrast this from \c strict, the \c decimal does not
         *  allow leading zeros on numbers.
        **/
        decimal,
        /** Strictly comply with the JSON specification for numbers -- no leading zeros! **/
        strict,
    };
    
public:
    /** Create an instance with the default options. **/
    parse_options();
    
    ~parse_options() noexcept;
    
    /** Create a parser with the default options -- this is the same result as the default constructor, but might be
     *  helpful if you like to be more explicit.
    **/
    static parse_options create_default();
    
    /** Create a strict parser. In general, these options are meant to fail on anything that is not a 100% valid JSON
     *  document. More specifically:
     *  
     *  \code
     *  failure_mode() == on_error::fail_immediately
     *  string_encoding() == encoding::utf8_strict
     *  number_encoding() == numbers::strict
     *  comma_policy() == commas::strict
     *  max_structure_depth() == 20
     *  require_document() == true
     *  complete_parse() == true
     *  comments() == false
     *  \endcode
    **/
    static parse_options create_strict();
    
    /** See \c on_error. The default failure mode is \c fail_immediately. **/
    on_error failure_mode() const;
    parse_options& failure_mode(on_error mode);
    
    /** The maximum allowed parsing failures the parser can encounter before throwing an error. This is only applicable
     *  if the \c failure_mode is not \c on_error::fail_immediately. By default, this value is 10.
     *  
     *  You should probably not set this value to an unreasonably high number, as each parse error encountered must be
     *  stored in memory for some period of time.
    **/
    std::size_t max_failures() const;
    parse_options& max_failures(std::size_t limit);
    
    /** The output encoding for multi-byte characters in strings. The default value is UTF-8 because UTF-8 is best. Keep
     *  in mind this changes the output encoding for \e all decoded strings. If you need mixed encodings, you must
     *  handle that in your application.
    **/
    encoding string_encoding() const;
    parse_options& string_encoding(encoding);
    
    /** How should a parser interpret numbers? By default, this is \c numbers::decimal, which allows any form of decimal
     *  input.
    **/
    numbers number_encoding() const;
    parse_options& number_encoding(numbers);
    
    /** How should extra commas be treated? By default, this is \c commas::allow_trailing. **/
    commas comma_policy() const;
    parse_options& comma_policy(commas);
    
    /** The maximum allowed nesting depth of any structure in the JSON document. The JSON specification technically
     *  limits the depth to 20, but very few implementations actually conform to this, so it is fairly dangerous to set
     *  this value. By default, the value is 0, which means we should not do any depth checking.
    **/
    size_type max_structure_depth() const;
    parse_options& max_structure_depth(size_type depth);
    
    /** If set to true, the result of a parse is required to have \c kind of \c kind::object or \c kind::array. By
     *  default, this is turned off, which will allow \c parse to return incomplete documents.
    **/
    bool require_document() const;
    parse_options& require_document(bool);
    
    /** Should the input be completely parsed to consider the parsing a success? This is on by default. Disabling this
     *  option can be useful for situations where JSON input is coming from some stream and you wish to process distinct
     *  objects separately (this technique is used to great effect in jq: http://stedolan.github.io/jq/).
     *  
     *  \warning
     *  When using this option, it is best to construct a \c tokenizer for your input stream and reuse that. The
     *  \c parse functions all internally buffer your \c istream and while they \e attempt to use \c putback re-put
     *  characters back into the \c istream, they are not necessarily successful at doing so.
    **/
    bool complete_parse() const;
    parse_options& complete_parse(bool);
    
    /** Are JSON comments allowed?
     *  
     *  \warning
     *  There is no "official" syntax for JSON comments, but this system allows
    **/
    bool comments() const;
    parse_options& comments(bool);
    
private:
    // For the purposes of ABI compliance, most modifications to the variables in this class should bump the minor
    // version number.
    on_error    _failure_mode     = on_error::fail_immediately;
    std::size_t _max_failures     = 10;
    encoding    _string_encoding  = encoding::utf8;
    numbers     _number_encoding  = numbers::decimal;
    commas      _comma_policy     = commas::allow_trailing;
    size_type   _max_struct_depth = 0;
    bool        _require_document = false;
    bool        _complete_parse   = true;
    bool        _comments         = true;
};

/** Reads a JSON value from the input stream.
 *  
 *  \note
 *  This function is \e not intended for verifying if the input is valid JSON, as it will intentionally correctly parse
 *  invalid JSON (so long as it resembles valid JSON). See \c parse_options::create_strict for a strict-mode parse.
 *  
 *  \example "parse(std::istream&, const parse_options&)"
 *  Parse JSON from some file.
 *  \code
 *  std::ifstream file("file.json");
 *  jsonv::value out = parse(file);
 *  \endcode
**/
value JSONV_PUBLIC parse(std::istream& input, const parse_options& = parse_options());

/** Construct a JSON value from the given input.
 *  
 *  \throws parse_error if an error is found in the JSON. If the \a input terminates unexpectedly, a \c parse_error will
 *   still be thrown with a message like "Unexpected end: unmatched {...". If you suspect the input of going bad, you
 *   can check the state flags or set the exception mask of the stream (exceptions thrown by \a input while processing
 *   will be propagated out).
**/
value JSONV_PUBLIC parse(const string_view& input, const parse_options& = parse_options());

/** Reads a JSON value from a buffered \c tokenizer. This less convenient function is useful when setting
 *  \c parse_options::complete_parse to \c false.
 *  
 *  \see parse(std::istream&, const parse_options&)
 *  
 *  \example "parse(tokenizer&, const parse_options&)"
 *  \code
 *  tcp_stream input(get_network_stream());
 *  jsonv::tokenizer buffered(input);
 *  jsonv::parse_options options = jsonv::parse_options().complete_parse(false);
 *  jsonv::value x = parse(buffered, options);
 *  jsonv::value y = parse(buffered, options);
 *  \endcode
**/
value JSONV_PUBLIC parse(tokenizer& input, const parse_options& = parse_options());

}

#endif/*__JSONV_PARSE_HPP_INCLUDED__*/
