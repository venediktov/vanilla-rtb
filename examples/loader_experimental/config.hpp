
#pragma once
#ifndef ICO_CACHE_LOADER_CONFIG_HPP
#define ICO_CACHE_LOADER_CONFIG_HPP

struct ico_cache_loader_config_data {
    std::string log_file_name;
    std::string domain_source;
    std::string domain_ipc_name;
    std::string ico_campaign_source;
    std::string ico_campaign_ipc_name;
    std::string ads_source;
    std::string ads_ipc_name;
    std::string campaign_budget_source;
    std::string ipc_name;
};

#endif /* ICO_CACHE_LOADER_CONFIG_HPP */

