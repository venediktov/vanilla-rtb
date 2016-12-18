//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modified by Vladimir Venediktov :
// adding template parameter for connection_ptr type , minimum changes required to work with 
// any handler, removing connection_manager.cpp from my project.
//
 
#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP
 
#include <set>
 
namespace http {
namespace server {
 
/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
template<typename connection_ptr>   
class connection_manager
{
public:
  connection_manager(const connection_manager&) = delete;
  connection_manager& operator=(const connection_manager&) = delete;
 
  /// Construct a connection manager.
  connection_manager() {}
 
  /// Add the specified connection to the manager and start it.
  void start(connection_ptr c)
  {
    connections_.insert(c);
    c->start();
  }
 
  /// Stop the specified connection.
  void stop(connection_ptr c)
  {
    connections_.erase(c);
    c->stop();
  }
 
  /// Stop all connections.
  void stop_all()
  {
    for (auto c: connections_)
      c->stop();
    connections_.clear();
  }
 
private:
  /// The managed connections.
  std::set<connection_ptr> connections_;
};
 
} // namespace server
} // namespace http
 
#endif // HTTP_CONNECTION_MANAGER_HPP
 
