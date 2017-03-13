

#ifndef CAMPAIGN_MANAGER_CONFIG_HPP
#define CAMPAIGN_MANAGER_CONFIG_HPP

struct campaign_manager_config_data {
    std::string log_file_name;
    std::string create_restful_prefix;
    std::string update_restful_prefix;
    std::string delete_restful_prefix;
    std::string ipc_name;
};
using CampaignManagerConfig = vanilla::config::config<campaign_manager_config_data>;

#endif /* CAMPAIGN_MANAGER_CONFIG_HPP */

