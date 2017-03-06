

#ifndef CACHE_LOADER_CONFIG_HPP
#define CACHE_LOADER_CONFIG_HPP

struct cache_loader_config_data {
    std::string log_file_name;
    std::string ads_source;
    std::string ads_ipc_name;
    std::string geo_ad_source;
    std::string geo_ad_ipc_name;
    std::string geo_source;
    std::string geo_ipc_name;   
};
using CacheLoadConfig = vanilla::config::config<cache_loader_config_data>;

#endif /* CACHE_LOADER_CONFIG_HPP */

