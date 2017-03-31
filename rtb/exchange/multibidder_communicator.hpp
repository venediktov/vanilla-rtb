/* 
 * File:   multibidder_communicator.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 31 марта 2017 г., 12:14
 */

#ifndef MULTIBIDDER_COMMUNICATOR_HPP
#define MULTIBIDDER_COMMUNICATOR_HPP

#include "rtb/messaging/communicator.hpp"
#include "rtb/exchange/multibidder_collector.hpp"
#include "rtb/core/openrtb.hpp"

namespace vanilla {

    class multibidder_communicator {
    public:
        multibidder_communicator(uint16_t bind_port, uint32_t response_timeout) :
            response_timeout(response_timeout) {
            communicator.outbound(bind_port);
        }

        void process(const vanilla::VanillaRequest &vanilla_request, multibidder_collector &collector) {
            communicator
                .distribute(vanilla_request)
                .collect<openrtb::BidResponse>(std::chrono::milliseconds(response_timeout), [&collector](openrtb::BidResponse bid, auto done) { //move ctored by collect()    
                    collector.add(std::move(bid));
                    if (collector.done()) {
                        done();
                    }
                });
        }
    private:
        vanilla::messaging::communicator<vanilla::messaging::broadcast> communicator;
        uint32_t response_timeout;
    };
}
#endif /* MULTIBIDDER_COMMUNICATOR_HPP */

