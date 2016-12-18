/*
 * File:   crud_matcher.hpp
* Author: vvenedict@gmail.com
*
* Created on September 29, 2015, 6:06 PM
*/
 
#ifndef _HTTP_CRUD_MATCHER_HPP__
#define                _HTTP_CRUD_MATCHER_HPP__
 
#include <functional>
#include <map>
#include <string>
 
namespace http { namespace crud {
    
    template<typename Matched>
    struct crud_match : Matched {
        crud_match(const Matched &m, const std::string &d) : Matched(m) , data(d) {}
        std::string data;
    };
    
    template<typename Response, typename Regex, typename Matched>
    struct crud_matcher {
        typedef std::function<void(Response &, const crud_match<Matched> &)>  request_handler_type;
        typedef crud_matcher<Response, Regex, Matched> self_type ;
        explicit crud_matcher(const Regex &expression) : _expression(expression) {}
        self_type & get(request_handler_type handler) {
            _handlers["GET"] = handler;
            return *this ;
        }
        self_type & post(request_handler_type handler) {
            _handlers["POST"] = handler;
            return *this;
        }
        self_type & del(request_handler_type handler) {
            _handlers["DELETE"] = handler;
            return *this;
        }
       self_type & put(request_handler_type handler) {
            _handlers["PUT"] = handler;
            return *this;
        }
        template<typename Request>
        void handle_request(const Request& request, Response& response, const Matched &what) {
             //dispatching to matching based on CRUD handler
            crud_match<Matched> match(what, request.data) ;
            _handlers[request.method](response, match) ;
        }
    private:
       Regex _expression;
       std::map<std::string, request_handler_type> _handlers;
    };
 
}}
 
#endif   /* __HTTP_CRUD_MATCHER__ */
