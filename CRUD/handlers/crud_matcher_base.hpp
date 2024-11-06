/*
* File:   crud_matcher_base.hpp
* Author: vvenedict@gmail.com
*
* Created on September 29, 2015, 6:06 PM
*/
#pragma once
#ifndef HTTP_CRUD_MATCHER_BASE_HPP
#define HTTP_CRUD_MATCHER_BASE_HPP
 
#include <functional>
#include <map>
#include <string>
 
namespace http::crud {

    template<typename Response, typename Matcher>
    struct crud_matcher_base {
        using request_handler_type = std::function<void(Response&, Matcher const&)>;
        explicit crud_matcher_base() = default;
        crud_matcher_base & get(request_handler_type handler) {
            _handlers["GET"] = handler;
            return *this;
        }
        crud_matcher_base  & post(request_handler_type handler) {
            _handlers["POST"] = handler;
            return *this;
        }
        crud_matcher_base  & del(request_handler_type handler) {
            _handlers["DELETE"] = handler;
            return *this;
        }
        crud_matcher_base  & put(request_handler_type handler) {
            _handlers["PUT"] = handler;
            return *this;
        }
        template<typename Request, typename Matched>
        void handle_request(const Request& request, Response& response, Matched const& what) {
            auto &handler = _handlers[request.method];
            Matcher matcher(what, request);
            if (handler) {
                handler(response, matcher);
            }
        }
    private:
      std::map<std::string, request_handler_type> _handlers;
    };
 
}
 
#endif   /* HTTP_CRUD_MATCHER_BASE_HPP */
