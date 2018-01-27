/*
 * File:  algos.hpp
 * Author: Vladimir Venediktov
 * Author: Arseny Bushev
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on January 8, 2018, 1:42 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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
