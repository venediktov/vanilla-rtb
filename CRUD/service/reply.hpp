//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modified by Vladimir Venediktov:
// Adding spec for stream operator

#ifndef HTTP_REPLY_HPP
#define HTTP_REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/serialization/strong_typedef.hpp>
#include "header.hpp"
#include "mime_types.hpp"

namespace http {
namespace server {

/// A reply to be sent to a client.
struct reply
{
  BOOST_STRONG_TYPEDEF(std::string, flush)  
  /// The status of the reply.
  enum status_type
  {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  } status;

  /// The headers to be included in the reply.
  std::vector<header> headers;

  /// The content to be sent in the reply.
  std::string content;

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> to_buffers();

  /// Get a stock reply.
  static reply stock_reply(status_type status, const char* mime = mime_types::HTML);

  /// flush function to be used by reply & operator<<(reply &r , const reply::flush &) 
  template<typename FlushT>
  static reply & flush_impl(reply &r, reply::status_type status, FlushT && f) {
    r.status = status;
    r.headers.resize(2);
    r.headers[0].name = "Content-Length";
    r.headers[0].value = std::to_string(r.content.size());
    r.headers[1].name = "Content-Type";
    r.headers[1].value = mime_types::extension_to_type(std::forward<FlushT>(f));
    return r;
  }

};

reply & operator<<(reply &r , const std::string &value) ;
reply & operator<<(reply &r , const reply::flush &) ;

} // namespace server
} // namespace http

#endif // HTTP_REPLY_HPP
