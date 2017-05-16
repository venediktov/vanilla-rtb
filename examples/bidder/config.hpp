/* 
 * File:   config.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:12
 */

#ifndef BIDDER_CONFIG_HPP
#define BIDDER_CONFIG_HPP

struct bidder_config_data {
    std::string log_file_name;
    std::string ads_source;
    std::string ads_ipc_name;
    std::string geo_ad_source;
    std::string geo_ad_ipc_name;
    std::string geo_source;
    std::string geo_ipc_name;
    std::string geo_campaign_ipc_name;
    std::string geo_campaign_source;
    std::string campaign_data_source;
    std::string campaign_data_ipc_name;
    std::string key_value_host;
    int key_value_port;
    int timeout;
    unsigned int concurrency;
    short port;
    std::string host;
    std::string root;
    short num_of_bidders;
    
    bidder_config_data() :
        log_file_name{}, 
        ads_source{}, ads_ipc_name{}, 
        geo_ad_source{}, geo_ad_ipc_name{},
        geo_source{}, geo_ipc_name{}, geo_campaign_ipc_name{},
        geo_campaign_source{},
        campaign_data_source{}, campaign_data_ipc_name{},
        key_value_host{}, key_value_port{}, 
        timeout{}, concurrency{},
        port{}, host{}, root{}, num_of_bidders{}
    {}
};
using BidderConfig = vanilla::config::config<bidder_config_data>;

#endif /* BIDDER_CONFIG_HPP */

