/* 
 * File:   user_info.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 10 марта 2017 г., 23:18
 */

#ifndef USER_INFO_HPP
#define USER_INFO_HPP

#include <string>
#include "rtb/core/bid_request.hpp"
#include "rtb/DSL/generic_dsl.hpp"

namespace vanilla {
    struct UserInfo {
        std::string user_id{};
        std::string user_data{};
    };
    
    using VanillaRequest = vanilla::BidRequest<DSL::GenericDSL<std::string>, vanilla::UserInfo>;
}



namespace boost { namespace serialization {
    template<class Archive>
    void serialize(Archive & ar, vanilla::UserInfo & value, const unsigned int version) {
        ar & value.user_id;
        ar & value.user_data;
    }
}}
#endif /* USER_INFO_HPP */

