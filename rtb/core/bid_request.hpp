/* 
 * File:   bid_request.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 10 марта 2017 г., 23:08
 */

#ifndef BID_REQUEST_HPP
#define BID_REQUEST_HPP

#include "openrtb.hpp"

namespace vanilla {
    template <typename UserInfo, typename T=std::string>
    struct BidRequest {
        using request_type = openrtb::BidRequest<T>;
        request_type bid_request;
        UserInfo user_info;
        
        BidRequest &operator=(const request_type &req) {
            bid_request = req;
            return *this;
        }
        
        const request_type& request() const {
            return bid_request;
        }
    };
}

#endif /* BID_REQUEST_HPP */

