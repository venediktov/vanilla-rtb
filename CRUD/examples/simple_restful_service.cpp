//
// resful_service.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modified by Vladimir Venediktov :
// Adding use case for Restful handlers, converting Web server into Retful Webservice
//
 
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "service/server.hpp"
#include "service/request_handler.hpp"
#include "service/request.hpp"
#include "service/reply.hpp"
#include "handlers/crud_dispatcher.hpp"

 
int main(int argc, char* argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 4)
    {
      std::cerr << "Usage: http_server <address> <port> <doc_root>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    receiver 0.0.0.0 80 .\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    receiver 0::0 80 .\n";
      return 1;
    }
 
   
    // SIMPLE NO REGEX MATCH FOR POST "/venue_handler/RTB"
    using restful_dispatcher_t = http::crud::crud_dispatcher<http::server::request, http::server::reply, std::string, std::string> ;

    restful_dispatcher_t handler(argv[3]) ;
    handler.crud_match(std::string("/venue_handler/RTB") )
        .get([](http::server::reply & r, const http::crud::crud_match<std::string> &) {
            //r = http::server::reply::stock_reply(http::server::reply::no_content, http::server::mime_types::JSON) ;
            r << "{value:\"Hello World\"}" << http::server::reply::flush("json") ;
            std::cout << "GET request" << std::endl;
        })
        .post([](http::server::reply & r, const http::crud::crud_match<std::string> & match) {
            r << "{}" << http::server::reply::flush("json") ;
            //r = http::server::reply::stock_reply(http::server::reply::no_content, http::server::mime_types::JSON) ;
            std::cout << "POST request_data=" << match.data << std::endl;
        });

    // Initialise the server.
    http::server::server<restful_dispatcher_t> s(argv[1], argv[2], handler);
    
    // Run the server until stopped.
    s.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }
 
  return 0;
}
