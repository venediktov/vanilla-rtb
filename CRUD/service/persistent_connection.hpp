/*
 * File:   persistent_connection.hpp
 * Author: Vladimir Venediktov
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on April 5, 2017, 5:13 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*
*/

#pragma once
#ifndef HTTP_PERSISTENT_CONNECTION_HPP
#define HTTP_PERSISTENT_CONNECTION_HPP


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
class persistent_connection
  : public std::enable_shared_from_this<persistent_connection<request_handler_type> >
{
    typedef persistent_connection<request_handler_type> self_type;
    typedef std::shared_ptr<self_type> self_type_ptr;
    typedef connection_manager<self_type_ptr> connection_manager_type;
public:
  persistent_connection(const persistent_connection&) = delete;
  persistent_connection& operator=(const persistent_connection&) = delete;
 
  /// Construct a connection with the given socket.
  explicit persistent_connection(boost::asio::ip::tcp::socket socket,
      connection_manager_type& manager, request_handler_type& handler)
  : socket_(std::move(socket)),
    connection_manager_(manager),
    request_handler_(handler)
  {
  }
 
  /// Start the first asynchronous operation for the persistent_connection.
  void start() {
    do_read();
  }
 
  /// Stop all asynchronous operations associated with the persistent_connection.
  void stop() {
   socket_.close();
  }
 
private:
  /// Perform an asynchronous read operation.
  void do_read()
  {
    socket_.async_read_some(boost::asio::buffer(buffer_),
        [this](boost::system::error_code ec, std::size_t bytes_transferred)
        {
            
          if (!ec) [[likely]]
          {
            request_parser::result_type result;
            char * data = nullptr;
            std::tie(result, data) = request_parser_.parse(
                request_, buffer_.data(), buffer_.data() + bytes_transferred);

            if (result == request_parser::good) [[likely]]
            {
              ec = read_data_if(data, bytes_transferred);
              if (!ec) [[likely]]
              {
                request_handler_.handle_request(request_, reply_);
              }
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
    reply_.headers.emplace_back("Connection" , "keep-alive");
    boost::asio::async_write(socket_, reply_.to_buffers(),
        [this](boost::system::error_code, std::size_t)
        {
          // TODO: VL: why is error_code not checked?
            request_parser_.reset();
            reply_.headers.resize(0);
            reply_.status = reply::status_type::ok;
            reply_.content="";
            request_ = request(); 
            do_read();
        });
  }

  boost::system::error_code read_data_if(char * const data, std::size_t bytes_transferred) {
    static constexpr auto is_content_length = [](const header &h) noexcept { return h.name == "content-length" ; };

    boost::system::error_code ec{};
    if (data != nullptr) {
        auto itr = find_if(begin(request_.headers), end(request_.headers), is_content_length);

        if (itr != end(request_.headers)) {
            auto received_data_size = std::distance(data, buffer_.data() + bytes_transferred);
            auto content_length_value = boost::lexical_cast<long>(itr->value);
            request_.data = std::string(data, content_length_value);
            if (received_data_size < content_length_value) {
                auto left_over_size = content_length_value - received_data_size;
                auto end_data_ptr = request_.data.data() + received_data_size;
                auto left_over_buffer = boost::asio::buffer(end_data_ptr, left_over_size);
                boost::asio::read(socket_, left_over_buffer, ec);
            }
        }
    }
    return ec;
  }

  /// Socket for the persistent_connection.
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
 
#endif // HTTP_PERSISTENT_CONNECTION_HPP
