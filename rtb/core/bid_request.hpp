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
    template <typename UserInfo>
    struct BidRequest {
        openrtb::BidRequest bid_request;
        UserInfo user_info;
    };
}

#endif /* BID_REQUEST_HPP */

