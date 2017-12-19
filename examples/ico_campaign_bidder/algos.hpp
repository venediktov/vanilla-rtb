//
// Created by vladimir venediktov on 12/18/17.
//

#ifndef VANILLA_RTB_ALGOS_HPP
#define VANILLA_RTB_ALGOS_HPP

#include <memory>

namespace vanilla { namespace algorithm {

        template<typename Ad>
        static auto calculate_max_bid(const std::vector<Ad>& ads) -> decltype(std::make_shared<Ad>()){
            if(ads.size() == 0) {
                return std::make_shared<Ad>();
            }
            const typename std::vector<Ad>::const_iterator result =
                std::max_element(ads.cbegin(), ads.cend(), [](const Ad &first, const Ad &second) -> bool {
                    return first.auth_bid_micros && second.auth_bid_micros ?
                           first.auth_bid_micros <second.auth_bid_micros : first.max_bid_micros < second.max_bid_micros;
                });
            return std::make_shared<Ad>(*result);
        }

}}


#endif //VANILLA_RTB_ALGOS_HPP
