/*
* File:   crud_matcher_with_request.hpp
* Author: vvenedict@gmail.com
*
* Created on September 29, 2015, 6:06 PM
*/
#pragma once
#ifndef HTTP_CRUD_MATCHER_WITH_REQUEST_HPP
#define HTTP_CRUD_MATCHER_WITH_REQUEST_HPP
 
#include <functional>
#include <map>
#include <string>

#include "crud_matcher_base.hpp"

namespace http::crud {

    template<typename Matched, typename Request>
    struct crud_matched_request : Matched {
        crud_matched_request(const Matched &m, const Request &request) : Matched(m) , request(request) {}
        Request request;
    };

    template<typename Request, typename Response, typename Regex, typename Matched>
    struct crud_matcher_with_request : crud_matcher_base<Response, crud_matched_request<Matched, Request>> {};
 
}
 
#endif   /* HTTP_CRUD_MATCHER_WITH_REQUEST_HPP */
