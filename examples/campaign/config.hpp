/*
* File:   serialization.hpp
* Author: Vladimir Venediktov
*
* Created on March 12, 2017, 10:25 PM
*/

#ifndef CAMPAIGN_CONFIG_HPP
#define CAMPAIGN_CONFIG_HPP

#include "rtb/config/config.hpp"

struct campaign_manager_config_data {
    std::string log_file_name;
    std::string create_restful_prefix;
    std::string update_restful_prefix;
    std::string delete_restful_prefix;
    std::string ipc_name;
};
using CampaignManagerConfig = vanilla::config::config<campaign_manager_config_data>;

struct notification_service_config_data {
    std::string log_file_name;
    int budget_port;
    std::string nurl_match;
    std::string ipc_name;
    
    notification_service_config_data() :
        log_file_name{}, budget_port{}, nurl_match{}
    {}
};

using WinNotificationConfig = vanilla::config::config<notification_service_config_data>;

#endif /* CAMPAIGN_CONFIG_HPP */

