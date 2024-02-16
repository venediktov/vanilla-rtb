//
// regex_resful_service.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
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
 
    // Initialise the server.
    typedef http::crud::crud_dispatcher<http::server::request, http::server::reply> restful_dispatcher_t;
    restful_dispatcher_t handler(argv[3]) ;
   
    // READ or CREAT/UPDATE "/venue_handler/XEMDP/123"
    handler.crud_match(boost::regex("/venue_handler/(\\w+)/(\\d+)") )
        .get([](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            r << "name: " << match[1] << ", instance number: " << match[2]
              << http::server::reply::flush("text") ;
            std::cout << "GET request=" << match[0] << std::endl;
        })
        .post([](http::server::reply & r, const http::crud::crud_match<boost::cmatch>  & match) {
            r << "name: " << match[1] << ", instance number: " << match[2]
              << http::server::reply::flush("test") ;
            std::cout << "POST request=" << match[0] << std::endl;
            std::cout << "POST request_data=" << match.data << std::endl;
        }) ;
       
    // READ "/venue_handler/FLO/"
    handler.crud_match(boost::regex("/venue_handler/(\\w+)") )
        .post([](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            r << "name: " << match[1]
              << http::server::reply::flush("text") ;
            std::cout << "POST request=" << match[0] << std::endl;
        });
       

    //Create service from handler
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
