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
#include "char_convert.hpp"

#include "detail/fixed_map.hpp"

/** \def JSONV_CHAR_CONVERT_USE_BOOST_LOCALE
 *  Should JSON Voorhees use Boost.Locale to perform character conversions instead of the C++ Standard Library's
 *  \c codecvt? This must be set in GCC versions before 5, since \c codecvt is not supported.
**/
#ifndef JSONV_CHAR_CONVERT_USE_BOOST_LOCALE
#   if defined __clang__ && (__clang_major__ < 4 || (__clang_major__ == 3 && __clang_minor__ < 5))
#       define JSONV_CHAR_CONVERT_USE_BOOST_LOCALE 1
#   elif defined __GNUC__ && __GNUC__ < 5
#       define JSONV_CHAR_CONVERT_USE_BOOST_LOCALE 1
#   elif defined _MSC_VER && _MSC_VER < 1900
#       define JSONV_CHAR_CONVERT_USE_BOOST_LOCALE 1
#   else
#       define JSONV_CHAR_CONVERT_USE_BOOST_LOCALE 0
#   endif
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>

#if JSONV_CHAR_CONVERT_USE_BOOST_LOCALE
#include <boost/locale.hpp>
#else
#include <codecvt>
#endif

namespace jsonv
{
namespace detail
{

decode_error::decode_error(size_type offset, const std::string& message):
        runtime_error(message),
        _offset(offset)
{ }

decode_error::~decode_error() noexcept
{ }

#define ESCAPES_LIST(item) \
    item('\b', 'b')  \
    item('\f', 'f')  \
    item('\n', 'n')  \
    item('\r', 'r')  \
    item('\t', 't')  \
    item('\\', '\\') \
    item('/',  '/')  \
    item('\"', '\"') \

#define TUPLE_PLUS_1_GEN(a, b) +1
typedef detail::fixed_map<char, char, ESCAPES_LIST(TUPLE_PLUS_1_GEN)> converter_map;

/** These entries are sorted by the numeric value of the ASCII character (\c less_entry_cpp).
 *  
 *  \note
 *  The encode and decode map must be in a different order (even though they contain the same data) because the ASCII
 *  representations of escape sequences are not in the same order as the characters they are escaping.
**/
#define TUPLE_FIRST_SECOND(a, b) { a, b },
const converter_map encode_map = { ESCAPES_LIST(TUPLE_FIRST_SECOND) };

/** These entries are sorted by the character value of the escape sequence (\c less_entry_json).
**/
#define TUPLE_SECOND_FIRST(a, b) { b, a },
const converter_map decode_map = { ESCAPES_LIST(TUPLE_SECOND_FIRST) };

const char* find(const converter_map& source, char key)
{
    converter_map::const_iterator iter = source.find(key);
    if (iter != source.end())
        return &iter->second;
    else
        return NULL;
}

static const char* find_encoding(char native_character)
{
    return find(encode_map, native_character);
}

static const char* find_decoding(char char_after_backslash)
{
    return find(decode_map, char_after_backslash);
}

static bool needs_unicode_escaping(char c)
{
    return bool(c & '\x80')
        || !std::isprint(c);
}

static constexpr bool char_bitmatch(char c, char pos, char neg)
{
    return (c & pos) == pos
        && !(c & neg);
}

/** Tests if \a c is a valid UTF-8 sequence continuation. **/
static bool is_utf8_sequence_continuation(char c)
{
    return char_bitmatch(c, '\x80', '\x40');
}

template <typename FOnError>
static void utf8_extract_info(char c, unsigned& length, char& bitmask, const FOnError& on_error)
{
    if (!(c & '\x80'))
    {
        length = 1;
        bitmask = '\x7f';
    }
    else if (char_bitmatch(c, '\xc0', '\x20'))
    {
        length = 2;
        bitmask = '\x1f';
    }
    else if (char_bitmatch(c, '\xe0', '\x10'))
    {
        length = 3;
        bitmask = '\x0f';
    }
    else if (char_bitmatch(c, '\xf0', '\x08'))
    {
        length = 4;
        bitmask = '\x07';
    }
    else if (char_bitmatch(c, '\xf8', '\x04'))
    {
        length = 5;
        bitmask = '\x03';
    }
    else if (char_bitmatch(c, '\xfc', '\x02'))
    {
        length = 6;
        bitmask = '\x01';
    }
    else
    {
        // This is not an acceptable/valid UTF8 string.  A failure here means I can't trust or don't understand the
        // source encoding.
        on_error(c);
        // In release mode, we'll spit out *potentially* garbage JSON.
        length = 1;
        bitmask = '\x7f';
    }
}
static bool utf8_extract_info(char c, unsigned& length, char& bitmask)
{
    bool rc = true;
    utf8_extract_info(c, length, bitmask, [&rc] (char) { rc = false; });
    return rc;
}

static bool utf8_extract_code(const char* c, unsigned length, char bitmask, char32_t& num)
{
    const char submask = '\x3f';
    
    num = char32_t(*c & bitmask);
    ++c;
    
    for (unsigned i = 1; i < length; ++i, ++c)
    {
        if (char_bitmatch(*c, '\x80', '\x40'))
        {
            num = char32_t(num << 6);
            num = char32_t(num | (*c & submask));
        }
        else
        {
            // bad encoding -- let the caller know
            return false;
        }
    }
    
    return true;
}

static const char hex_codes[] = "0123456789abcdef";

static void to_hex(std::ostream& stream, uint16_t code)
{
    for (int pos = 3; pos >= 0; --pos)
    {
        uint16_t local_code = (code >> (4 * pos)) & uint16_t(0x000f);
        stream << hex_codes[local_code];
    }
}

static void utf16_create_surrogates(char32_t codepoint, uint16_t* high, uint16_t* low)
{
    // surrogate pair generation is slightly insane
    //   0000 0000 0000 nnnn  nnnn nnnn nnnn nnnn
    // - 0000 0000 0000 0001  0000 0000 0000 0000
    // ==========================================
    //   1101 10aa aaaa aaaa  1101 11bb bbbb bbbb
    // | high               | low                |
    uint32_t val = codepoint - 0x10000;
    *high = uint16_t(val >> 10)    | 0xd800;
    *low  = uint16_t(val & 0x03ff) | 0xdc00;
}

std::ostream& string_encode(std::ostream& stream, string_view source, bool ensure_ascii)
{
    typedef string_view::size_type size_type;
    
    for (size_type idx = 0, source_size = source.size(); idx < source_size; /* incremented inline */)
    {
        const char& current = source[idx];
        if (const char* replacement = find_encoding(current))
        {
            stream << "\\" << *replacement;
            ++idx;
        }
        else
        {
            unsigned length;
            char bitmask;
            bool valid_utf8 = utf8_extract_info(current, length, bitmask);
            
            if (!needs_unicode_escaping(current))
            {
                stream << current;
            }
            else
            {
                char32_t code;
                if (!valid_utf8
                   || idx + length > source_size
                   || !utf8_extract_code(&current, length, bitmask, code)
                   )
                {
                    // Invalid UTF-8 encoding -- we're either at the end of the string or the bytes were not a valid
                    // UTF-8 sequence. In either case, we will drop in a numeric encoding (\u00NN) for the bytes.
                    length = 1;
                    code = char32_t(current) & 0xff;
                }
                
                // if the input string is valid UTF-8, let it pass through
                if (valid_utf8 && !ensure_ascii)
                {
                    stream.write(&current, length);
                }
                // basic multilingual plane points are encoded in hex
                else if (code < 0x10000)
                {
                    stream << "\\u";
                    to_hex(stream, uint16_t(code));
                }
                // Codepoints not in the basic multilingual plane must be encoded as surrogate pairs
                else
                {
                    uint16_t high, low;
                    utf16_create_surrogates(code, &high, &low);
                    stream << "\\u";
                    to_hex(stream, high);
                    stream << "\\u";
                    to_hex(stream, low);
                }
            }
            
            idx += length;
        }
    }
    
    return stream;
}

static uint16_t from_hex_digit(char c, std::size_t idx)
{
    switch (c)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    // The JSON spec isn't clear if it wants capitals or lowercase, so I'll just assume both are okay
    case 'a':
    case 'A':
        return 0xa;
    case 'b':
    case 'B':
        return 0xb;
    case 'c':
    case 'C':
        return 0xc;
    case 'd':
    case 'D':
        return 0xd;
    case 'e':
    case 'E':
        return 0xe;
    case 'f':
    case 'F':
        return 0xf;
    default:
        throw decode_error(idx, std::string("The character '") + c + "' is not a valid hexidecimal digit.");
    }
}

static uint16_t from_hex(const char* s, std::size_t idx_base)
{
    uint16_t x = 0U;
    for (int idx = 3; idx >= 0; --idx)
    {
        x = uint16_t(x + (from_hex_digit(*s, idx_base + idx) << (idx * 4)));
        ++s;
    }
    
    return x;
}

static void utf8_sequence_info(char32_t val, std::size_t* length, char* first)
{
    if (val < 0x00000080U)
    {
        *length = 1;
        *first = char(val);
    }
    else if (val < 0x00000800U)
    {
        *length = 2;
        *first = char('\xc0' | ('\x1f' & (val >> 6)));
    }
    else if (val < 0x00010000U)
    {
        *length = 3;
        *first = char('\xe0' | ('\x0f' & (val >> 12)));
    }
    else if (val < 0x00200000U)
    {
        *length = 4;
        *first = char('\xf0' | ('\x07' & (val >> 18)));
    }
    else if (val < 0x04000000U)
    {
        *length = 5;
        *first = char('\xf8' | ('\x03' & (val >> 24)));
    }
    else
    {
        *length = 6;
        *first = char('\xfc' | char('\x01' & (val >> 30)));
    }
}

static void utf8_append_code(std::string& str, char32_t val)
{
    char c;
    std::size_t length;
    utf8_sequence_info(val, &length, &c);
    
    char buffer[8];
    char* bufferOut = buffer;
    *bufferOut++ = c;
    
    std::size_t shift = (length - 2) * 6;
    for (std::size_t idx = 1; idx < length; ++idx)
    {
        c = char('\x80' | ('\x3f' & (val >> shift)));
        *bufferOut++ = c;
        shift -= 6;
    }
    
    str.append(buffer, bufferOut);
}

static bool utf16_combine_surrogates(uint16_t high, uint16_t low, char32_t* out)
{
    if ((high & 0xfc00U) != 0xd800 || (low & 0xfc00U) != 0xdc00)
    {
        // invalid surrogate pair
        return false;
    }
    else
    {
        // surrogate pairs take the form:
        // | high               | low                |
        //  1101 10aa aaaa aaaa  1101 11bb bbbb bbbb
        // result:
        //   0000 0000 0000 aaaa  aaaa aabb bbbb bbbb
        // + 0000 0000 0000 0001  0000 0000 0000 0000
        *out = 0x10000
             + (((char32_t(high) & 0x03ff) << 10) | (char32_t(low)  & 0x03ff));
        return true;
    }
}

/**
 *  \tparam require_printable Requires all characters in the sequence to be "printable" (aka: call \c std::isprint on
 *                            them). This will probably eventually eventually transform into a "strict mode."
**/
template <parse_options::encoding encoding, bool require_printable>
std::string string_decode(string_view source)
{
    typedef std::string::size_type size_type;
    
    std::string output;
    const char* last_pushed_src = source.data();
    size_type utf8_sequence_start = 0;
    unsigned remaining_utf8_sequence = 0;
    
    for (size_type idx = 0; idx < source.size(); /* incremented inline */)
    {
        const char& current = source[idx];
        if (remaining_utf8_sequence == 0)
        {
            if (current == '\\')
            {
                output.append(last_pushed_src, source.data()+idx);
                
                const char& next = source[idx + 1];
                if (const char* replacement = find_decoding(next))
                {
                    output += *replacement;
                    idx += 2;
                }
                else if (next == 'u')
                {
                    if (idx + 6 > source.size())
                        throw decode_error(idx, "unterminated Unicode escape sequence (must have 4 hex characters)");
                    uint16_t hexval = from_hex(&source[idx + 2], idx + 2);
                    
                    if (encoding == parse_options::encoding::cesu8 || hexval < 0xd800U || hexval > 0xdfffU)
                    {
                        utf8_append_code(output, hexval);
                        
                        idx += 6;
                    }
                    // numeric encoding is in U+d800 - U+dfff with UTF-8 output, so deal with surrogate pairing...
                    else
                    {
                        auto surrogateString = [&] () { return std::string(source.data()+idx, 6); };
                        if (  idx + 12 > source.size()
                           || idx +  8 > source.size()
                           || source[idx + 6] != '\\'
                           || source[idx + 7] != 'u'
                           )
                            throw decode_error(idx, std::string("unpaired high surrogate (") + surrogateString() + ")");
                        uint16_t hexlowval = from_hex(&source[idx + 8], idx + 8);
                        char32_t codepoint;
                        if (!utf16_combine_surrogates(hexval, hexlowval, &codepoint))
                            throw decode_error(idx, std::string("unpaired high surrogate (") + surrogateString() + ")");
                        
                        utf8_append_code(output, codepoint);
                        
                        idx += 12;
                    }
                }
                else
                {
                    throw decode_error(idx, std::string("Unknown escape character: ") + next);
                    //output += '?'; Maybe better solution if we don't want to throw
                    //++idx;
                }
                
                last_pushed_src = source.data() + idx;
            }
            else 
            {
                unsigned utf8_length;
                char utf8_bitmask;
                utf8_extract_info(current,
                                  utf8_length,
                                  utf8_bitmask,
                                  [&idx] (char x)
                                  {
                                      std::ostringstream os;
                                      os << "Invalid UTF-8 code point: \\x"
                                         << std::hex << std::setw(2) << static_cast<int>(x) << std::dec << '.';
                                      throw decode_error(idx, os.str());
                                  }
                                 );
                
                if (utf8_length > 1)
                {
                    utf8_sequence_start = idx;
                    remaining_utf8_sequence = utf8_length - 1;
                    ++idx;
                }
                else if (require_printable && !std::isprint(current))
                {
                    std::ostringstream os;
                    os << "Unprintable character found in input: ";
                    switch (current)
                    {
                    case '\t': os << "\\t (tab)"; break;
                    case '\b': os << "\\b (backspace)"; break;
                    case '\f': os << "\\f (formfeed)"; break;
                    case '\n': os << "\\n (newline)"; break;
                    case '\r': os << "\\r (carriage return)"; break;
                    default:   os << "\\x" << std::hex << std::setw(2) << static_cast<int>(current) << std::dec; break;
                    }
                    throw decode_error(idx, os.str());
                }
                else
                {
                    ++idx;
                }
            }
        }
        // remaining_utf8_sequence > 0
        else
        {
            if (is_utf8_sequence_continuation(current))
            {
                ++idx;
                --remaining_utf8_sequence;
            }
            // not on a UTF8 continuation, even though we should be...
            else
            {
                std::ostringstream os;
                os << "Invalid UTF-8 multi-byte sequence in source: \"";
                for (size_type pos = utf8_sequence_start; pos <= idx; ++pos)
                    os << "\\x" << std::setw(2) << std::hex << static_cast<int>(source[pos]);
                os << std::dec << "\". ";
                os << "The sequence should continue for " << remaining_utf8_sequence
                   << " character" << (remaining_utf8_sequence == 1 ? "" : "s");
                throw decode_error(idx, os.str());
            }
        }
    }
    
    if (encoding != parse_options::encoding::cesu8 && remaining_utf8_sequence > 0)
    {
        std::ostringstream os;
        os << "unterminated UTF-8 sequence at end of string: \"";
        os << std::hex;
        for (size_type idx = utf8_sequence_start; idx < source.size(); ++idx)
        {
            os << "\\x" << std::setfill('0') << std::setw(2) << unsigned(int(source[idx]));
        }
        os << '\"';
        throw decode_error(utf8_sequence_start, os.str());
    }
    
    output.append(last_pushed_src, source.end());
    return output;
}

string_decode_fn get_string_decoder(parse_options::encoding encoding)
{
    switch (encoding)
    {
    case parse_options::encoding::cesu8:
        return string_decode<parse_options::encoding::cesu8, false>;
    case parse_options::encoding::utf8_strict:
        return string_decode<parse_options::encoding::utf8, true>;
    case parse_options::encoding::utf8:
    default:
        return string_decode<parse_options::encoding::utf8, false>;
    };
}

#if JSONV_CHAR_CONVERT_USE_BOOST_LOCALE

std::wstring convert_to_wide(string_view source)
{
    return boost::locale::conv::to_utf<wchar_t>(source.data(), source.data() + source.size(), "utf-8");
}

std::string convert_to_narrow(const std::wstring& source)
{
    return boost::locale::conv::from_utf(source, "utf-8");
}

std::string convert_to_narrow(const wchar_t* source)
{
    return boost::locale::conv::from_utf(source, "utf-8");
}

#else

std::wstring convert_to_wide(string_view source)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(source.data(), source.data() + source.size());
}

std::string convert_to_narrow(const std::wstring& source)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(source);
}

std::string convert_to_narrow(const wchar_t* source)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(source);
}

#endif

}
}
