/* 
 * File:   multiexchange_config.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 25 марта 2017 г., 13:17
 */

#ifndef MULTIEXCHANGE_STATUS_HPP
#define MULTIEXCHANGE_STATUS_HPP

#include <boost/atomic/atomic.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

namespace vanilla {
    namespace multiexchange {
        struct multi_exchange_status {
            boost::posix_time::ptime start{boost::posix_time::microsec_clock::local_time()};
            boost::atomic_uint64_t request_count{};
            boost::atomic_uint64_t all_response_count{};
            boost::atomic_uint64_t bidder_response_count{};
            boost::atomic_uint64_t empty_response_count{};
            boost::atomic_uint64_t timeout_response_count{};

            friend std::ostream& operator<<(std::ostream &os, const multi_exchange_status &st) {
                boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - st.start;
                os << "start: " << boost::posix_time::to_simple_string(st.start) << "<br/>" <<
                    "elapsed: " << boost::posix_time::to_simple_string(td) << "<br/>" <<
                    "<table border=0>" <<
                    "<tr><td>requests</td><td>" << st.request_count << "</td></tr>" <<
                    "<tr><td>all bidders responsed</td><td>" << st.all_response_count << "</td></tr>" <<
                    "<tr><td>bid responses</td><td>" << st.bidder_response_count << "</td></tr>" <<
                    "<tr><td>empty responses</td><td>" << st.empty_response_count << "</td></tr>" <<
                    "<tr><td>timeout responses</td><td>" << st.timeout_response_count << "</td></tr>" <<
                    "</table> ";
                return os;
            }

            std::string to_string() const {
                std::stringstream ss;
                ss << *this;
                return ss.str();
            }
        };
    }
}
#endif /* MULTIEXCHANGE_STATUS_HPP */

