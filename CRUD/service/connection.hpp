//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
// Modified by Vladimir Venediktov :
// 1.) Adding template paramter to work with any request_handler , not just created by M. Kohlhoff HTTP request handler
// connection.cpp no longer needed in  my project 
// 2.) Adding support for POST std::tie(result, std::ignore) --> std::tie(result, data) , and 
//     populating request_.data see find_if search for "Content-Type"
 
#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP
 
#include <array>
#include <memory>
#include <utility>
#include <iterator>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_parser.hpp"
 
namespace http {
namespace server {
 
template<class> class connection_manager;
 
/// Represents a single connection from a client.
template <typename request_handler_type>
class connection
  : public std::enable_shared_from_this<connection<request_handler_type> >
{
    typedef connection<request_handler_type> self_type;
    typedef std::shared_ptr<self_type> self_type_ptr;
    typedef connection_manager<self_type_ptr> connection_manager_type;
public:
  connection(const connection&) = delete;
  connection& operator=(const connection&) = delete;
 
  /// Construct a connection with the given socket.
  explicit connection(boost::asio::ip::tcp::socket socket,
      connection_manager_type& manager, request_handler_type& handler)
  : socket_(std::move(socket)),
    connection_manager_(manager),
    request_handler_(handler)
  {
  }
 
  /// Start the first asynchronous operation for the connection.
  void start() {
    do_read();
  }
 
  /// Stop all asynchronous operations associated with the connection.
  void stop() {
   socket_.close();
  }
 
private:
  /// Perform an asynchronous read operation.
  void do_read()
  {
    auto self(this->shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer_),
        [=](boost::system::error_code ec, std::size_t bytes_transferred)
        {
          if (!ec)
          {
            request_parser::result_type result;
            char * data;
            std::tie(result, data) = request_parser_.parse(
                request_, buffer_.data(), buffer_.data() + bytes_transferred);
            auto itr = std::find_if( request_.headers.begin(), 
                                     request_.headers.end(), 
                                     [](const header &h) { return h.name == "content-length" ; }
            ) ;
            if ( itr != request_.headers.end()) {
                request_.data = std::string(data, boost::lexical_cast<long>(itr->value)) ;
            }
            if (result == request_parser::good)
            {
              request_handler_.handle_request(request_, reply_);
              do_write();
            }
            else if (result == request_parser::bad)
            {
              reply_ = reply::stock_reply(reply::bad_request);
              do_write();
            }
            else
            {
              do_read();
            }
          }
          else if (ec != boost::asio::error::operation_aborted)
          {
            connection_manager_.stop(this->shared_from_this());
          }
        });
   }
 
  /// Perform an asynchronous write operation.
  void do_write()
  {
    auto self(this->shared_from_this());
    boost::asio::async_write(socket_, reply_.to_buffers(),
        [=](boost::system::error_code ec, std::size_t)
        {
          if (!ec)
          {
            // Initiate graceful connection closure.
            boost::system::error_code ignored_ec;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
              ignored_ec);
          }
 
          if (ec != boost::asio::error::operation_aborted)
          {
            connection_manager_.stop(this->shared_from_this());
          }
        });
  }
 
  /// Socket for the connection.
  boost::asio::ip::tcp::socket socket_;
 
  /// The manager for this connection.
  connection_manager_type& connection_manager_;
 
 /// The handler used to process the incoming request.
  request_handler_type& request_handler_;
 
  /// Buffer for incoming data.
  std::array<char, 8192> buffer_;
 
  /// The incoming request.
  request request_;
 
  /// The parser for the incoming request.
  request_parser request_parser_;
 
  /// The reply to be sent back to the client.
  reply reply_;
};
 
 
} // namespace server
} // namespace http
 
#endif // HTTP_CONNECTION_HPP
 
