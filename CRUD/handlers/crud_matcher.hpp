/*
 * File:   crud_matcher.hpp
* Author: vvenedict@gmail.com
*
* Created on September 29, 2015, 6:06 PM
*/
#pragma once
#ifndef HTTP_CRUD_MATCHER_HPP
#define HTTP_CRUD_MATCHER_HPP

#include <functional>
#include <map>
#include <string>
#include "crud_matcher_base.hpp"


namespace http::crud {

    template<typename Matched>
    struct crud_match : Matched {
        template<typename Request>
        crud_match(const Matched &m, const Request &request) : Matched(m) , data(request.data) {}
        std::string data;
    };

    template<typename Response, typename Regex, typename Matched>
    struct crud_matcher :  crud_matcher_base<Response, crud_match<Matched>> {};

}

#endif   /* HTTP_CRUD_MATCHER_HPP */
