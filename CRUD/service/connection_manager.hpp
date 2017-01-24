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
// 1.) adding template parameter for connection_ptr type , minimum changes required to work with 
// any handler, removing connection_manager.cpp from my project.
// 2.) adding multi-threading support spin_lock/std::mutex conditional compilation
 
#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP
 
#include <set>
#include <mutex>
#if defined(CRUD_SPIN_LOCK)
#include "spin_lock.hpp"
#endif
 
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
    std::lock_guard<connection_lock_type> guard(lock_);
    {
        connections_.insert(c);
    }
    c->start();
  }
 
  /// Stop the specified connection.
  void stop(connection_ptr c)
  {
    std::lock_guard<connection_lock_type> guard(lock_);
    {
        connections_.erase(c);
    }
    c->stop();
  }
 
  /// Stop all connections.
  void stop_all()
  {
    for (auto c: connections_)
      c->stop();
    std::lock_guard<connection_lock_type> guard(lock_);
    connections_.clear();
  }
 
private:
  /// The managed connections.
  std::set<connection_ptr> connections_;
  /// adding multi-threading support
#if defined(CRUD_SPIN_LOCK)
  using connection_lock_type = spin_lock;
#else
  using connection_lock_type = std::mutex;
#endif  
  connection_lock_type lock_;
};
 
} // namespace server
} // namespace http
 
#endif // HTTP_CONNECTION_MANAGER_HPP
 
