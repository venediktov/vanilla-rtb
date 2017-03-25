/* 
 * File:   multiexchange_config.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 25 марта 2017 г., 13:19
 */

#ifndef MULTIEXCHANGE_CONFIG_HPP
#define MULTIEXCHANGE_CONFIG_HPP

#include <string>
#include "../../rtb/config/config.hpp"

namespace vanilla {
    namespace multiexchange {

        struct multi_exchange_handler_config_data {
            std::string log_file_name;
            int handler_timeout;
            int num_bidders;
            int bidders_port;
            int bidders_response_timeout;
            int concurrency;


            multi_exchange_handler_config_data() :
                log_file_name{}, handler_timeout{}, num_bidders{}, bidders_port{}, bidders_response_timeout{}, concurrency{}
            {
            }
        };

        using multiexchange_config = vanilla::config::config<multi_exchange_handler_config_data>;
    }
}
#endif /* MULTIEXCHANGE_CONFIG_HPP */

