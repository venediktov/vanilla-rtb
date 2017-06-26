//
// main.cpp
// ~~~~~~~~
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
#include "server.hpp"
#include "request_handler.hpp"
#include "../handlers/crud_dispatcher.hpp"
#include "request.hpp"
#include "reply.hpp"

 
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
//    http::server::request_handler handler(argv[3]);
//    http::server::server<http::server::request_handler> s(argv[1], argv[2], handler);
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
       
    // SIMPLE NO REGEX MATCH FOR POST "/venue_handler/RTB"
    using simple_restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply, std::string, std::string> ;
    simple_restful_dispatcher_t simple_handler(argv[3]) ;
    simple_handler.crud_match(std::string("/venue_handler/RTB") )
        .post([](http::server::reply & r, const http::crud::crud_match<std::string> & match) {
            r << "{}" << http::server::reply::flush("json") ;
            //r = http::server::reply::stock_reply(http::server::reply::no_content, http::server::mime_types::JSON) ;
            std::cout << "POST request_data=" << match.data << std::endl;
        });

    //http::server::server<restful_dispatcher_t> s(argv[1], argv[2], simple_handler);
    http::server::server<simple_restful_dispatcher_t> s(argv[1], argv[2], simple_handler);
 
   
    
    // Run the server until stopped.
    s.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }
 
  return 0;
}
