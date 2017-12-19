/* 
 * File:   config.hpp
 */

#ifndef ICO_BIDDER_CONFIG_HPP
#define ICO_BIDDER_CONFIG_HPP

struct ico_bidder_config_data {
    std::string log_file_name;
    std::string ads_source;
    std::string ads_ipc_name;
    std::string referer_source;
    std::string referer_ipc_name;
    std::string ico_campaign_ipc_name;
    std::string ico_campaign_source;
    std::string campaign_budget_source;
    std::string ipc_name;
    int timeout;
    unsigned int concurrency;
    short port;
    std::string host;
    std::string root;
    short num_of_bidders;
    
    ico_bidder_config_data() : timeout{}, concurrency{}, port{}, num_of_bidders{}
    {}
};

#endif /* ICO_BIDDER_CONFIG_HPP */

