/* 
 * File:   multibidder_communicator.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 31 марта 2017 г., 12:14
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

#ifndef MULTIBIDDER_COMMUNICATOR_HPP
#define MULTIBIDDER_COMMUNICATOR_HPP

#include <chrono>
#include "rtb/messaging/communicator.hpp"
#include "rtb/exchange/multibidder_collector.hpp"
#include "rtb/core/openrtb.hpp"

namespace vanilla {

    template
    <  
        typename DSL,
        typename Duration  = std::chrono::milliseconds, 
        typename DeliveryType = vanilla::messaging::broadcast
    > 
    class multibidder_communicator {
    private:
            using serialized_type = typename DSL::serialized_type;
    public:
        multibidder_communicator(uint16_t bidders_port, Duration response_timeout) :
            response_timeout(response_timeout) {
            communicator.outbound(bidders_port);
        }

        template<typename Request>
        void process(const Request &request, multibidder_collector<typename serialized_type::data_type> &collector) {
            communicator
                .distribute(request)
                .template collect<serialized_type>(response_timeout, [&collector](serialized_type bid, auto done) { //move ctored by collect()    
                    collector.add(std::move(bid));
                    if (collector.done()) {
                        done();
                    }
                });
        }
    private:
        vanilla::messaging::communicator<DeliveryType> communicator;
        Duration response_timeout;
    };
}
#endif /* MULTIBIDDER_COMMUNICATOR_HPP */

