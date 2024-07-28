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

namespace http::server {

template<class> class connection_manager;

/// Result of data read.
enum data_read_result { complete, indeterminate };

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
    auto self{this->shared_from_this()}; //make sure connection is alive while lambda is in progress
    socket_.async_read_some(boost::asio::buffer(buffer_),
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
        {
            
          if (!ec) [[likely]]
          {
            request_parser::result_type result;
            char * data = nullptr;
            std::tie(result, data) = request_parser_.parse(
                request_, buffer_.data(), buffer_.data() + bytes_transferred);

            if (result == request_parser::good) [[likely]]
            {
              auto status = handle_request_if(data, bytes_transferred);
              if (status == data_read_result::complete) {
                request_handler_.handle_request(request_, reply_);
                do_write();
              }
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
    auto self{this->shared_from_this()}; //make sure connection is alive while lambda is in progress
    reply_.headers.emplace_back("Connection" , "keep-alive");
    boost::asio::async_write(socket_, reply_.to_buffers(),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            request_parser_.reset();
            reply_.headers.resize(0);
            reply_.status = reply::status_type::ok;
            reply_.content="";
            request_ = request();
            if(!ec) [[likely]] {
                do_read();
            } else if (ec != boost::asio::error::operation_aborted) {
                connection_manager_.stop(this->shared_from_this());
            }
        });
  }

  data_read_result handle_request_if(char * const data, std::size_t bytes_transferred) {
    static constexpr auto is_content_length = [](const header &h) noexcept { return h.name == "content-length" ; };

    data_read_result completion_status{data_read_result::complete};
    if (data != nullptr) {
        auto itr = find_if(begin(request_.headers), end(request_.headers), is_content_length);

        if (itr != end(request_.headers)) {
            auto received_data_size = std::distance(data, buffer_.data() + bytes_transferred);
            auto content_length_value = boost::lexical_cast<long>(itr->value);
            request_.data = std::string(data, received_data_size);
            if (received_data_size < content_length_value) {
                completion_status = data_read_result::indeterminate;
                request_.data.resize(content_length_value);
                auto left_over_size = content_length_value - received_data_size;
                auto end_data_ptr = request_.data.data() + received_data_size;
                auto left_over_buffer = boost::asio::buffer(end_data_ptr, left_over_size);
                auto self{this->shared_from_this()}; //make sure connection is alive while lambda is in progress
                boost::asio::async_read(socket_, left_over_buffer, boost::asio::transfer_exactly(left_over_size),
                                        [this, self](boost::system::error_code ec, std::size_t) {
                                            if (!ec) [[likely]] {
                                                request_handler_.handle_request(request_, reply_);
                                                do_write();
                                            } else if (ec != boost::asio::error::operation_aborted) {
                                                connection_manager_.stop(this->shared_from_this());
                                            }
                                        });
            }
        }
    }
    return completion_status;
  }

  /// Socket for the persistent_connection.
  boost::asio::ip::tcp::socket socket_;
 
  /// The manager for this connection.
  connection_manager_type& connection_manager_;
 
 /// The handler used to process the incoming request.
  request_handler_type& request_handler_;
 
  /// Buffer for incoming data.
  std::array<char, 8192> buffer_{};
 
  /// The incoming request.
  request request_;
 
  /// The parser for the incoming request.
  request_parser request_parser_;
 
  /// The reply to be sent back to the client.
  reply reply_{.status=reply::not_found, .headers={}, .content={}};
  
};
} // namespace http::server
#endif // HTTP_PERSISTENT_CONNECTION_HPP
