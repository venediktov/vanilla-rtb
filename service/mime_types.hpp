//
// mime_types.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modified by: Vladimir Venediktov
// Introduced constexpr for compile time use case
//

#ifndef HTTP_MIME_TYPES_HPP
#define HTTP_MIME_TYPES_HPP

#include <string>

namespace http {
namespace server {
namespace mime_types {

struct mapping
{
  const char* extension;
  const char* mime_type;
} constexpr mappings[] =
{
  { "gif", "image/gif" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "jpg", "image/jpeg" },
  { "png", "image/png" },
  { "json", "application/json"},
  { "css", "text/css"}
};


constexpr bool equal( char const* lhs, char const* rhs )
{
    while (*lhs || *rhs)
        if (*lhs++ != *rhs++)
            return false;
    return true;
}

constexpr const char* extension_to_type(const char* extension)
{
  for (mapping m: mappings)
  {
    if (equal(m.extension,extension) )
    {
      return m.mime_type;
    }
  }

  return "text/plain";
}

constexpr const char* GIF  = mime_types::extension_to_type("gif");
constexpr const char* HTM  = mime_types::extension_to_type("htm");
constexpr const char* HTML = mime_types::extension_to_type("html");
constexpr const char* JPG  = mime_types::extension_to_type("jpg");
constexpr const char* PNG  = mime_types::extension_to_type("png");
constexpr const char* JSON = mime_types::extension_to_type("json");
constexpr const char* CSS  = mime_types::extension_to_type("css");

/// Convert a file extension into a MIME type.
std::string extension_to_type(const std::string& extension);

} // namespace mime_types
} // namespace server
} // namespace http

#endif // HTTP_MIME_TYPES_HPP
