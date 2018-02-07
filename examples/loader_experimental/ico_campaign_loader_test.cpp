
// Work in progress ....

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include "CRUD/service/server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "rtb/config/config.hpp"
#include "datacache/ad_entity.hpp"
#include "datacache/geo_entity.hpp"
#include "datacache/city_country_entity.hpp"
#include "datacache/entity_cache.hpp"
#include "datacache/memory_types.hpp"
#include "rtb/common/perf_timer.hpp"
#include "bidder/bidder_caches.hpp"
#include "bidder/serialization.hpp"
#include "config.hpp" 


#include "rtb/core/core.hpp"

extern void init_framework_logging(const std::string &) ;


int main(int argc, char *argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using CacheLoadConfig = vanilla::config::config<ico_cache_loader_config_data>;
    
    CacheLoadConfig config([](ico_cache_loader_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("ico-cache-loader.log", boost::program_options::value<std::string>(&d.log_file_name), "ico_cache_loader_test log file name log")
            ("ico-cache-loader.host", "cache_loader_test Host")
            ("ico-cache-loader.port", "cache_loader_test Port")
            ("ico-cache-loader.root", "cache_loader_test Root")
            ("datacache.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("bidder/data/ads"), "ads_source file name")
            ("datacache.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
            ("datacache.ico_ref_source", boost::program_options::value<std::string>(&d.ico_ref_source)->default_value("bidder/data/ico_referer"), "ico referer source file name")
            ("datacache.ico_ref_ipc_name", boost::program_options::value<std::string>(&d.ico_ref_ipc_name)->default_value("vanilla-ico-ref-ipc"), "ico referer ipc name")        
            ("ico-bidder.ico_campaign_ipc_name", boost::program_options::value<std::string>(&d.ico_campaign_ipc_name)->default_value("vanilla-ico-campaign-ipc"), "ico campaign ipc name")
            ("ico-bidder.ico_campaign_source", boost::program_options::value<std::string>(&d.ico_campaign_source)->default_value("data/ico_campaign"), "ico_campaign_source file name")
            ("campaign-manager.ipc_name", boost::program_options::value<std::string>(&d.ipc_name),"campaign_budget IPC name")
            ("campaign-manager.budget_source", boost::program_options::value<std::string>(&d.campaign_budget_source)->default_value("data/campaign_budget"),"campaign_budget source file name")
        ;
    });
    
    try {
        config.parse(argc, argv);
    }
    catch(std::exception const& e) {
        LOG(error) << e.what();
        return 0;
    }
    LOG(debug) << config;
    init_framework_logging(config.data().log_file_name);
/***
    vanilla::BidderCaches<ICOCacheLoadConfig> bidder_caches(config);
    GeoAdDataEntity<CacheLoadConfig>  geo_ad_cache(config);
    GeoDataEntity<CacheLoadConfig>    ico_ref_cache(config);
    AdDataEntity<CacheLoadConfig>     ad_cache(config);
    
    std::map<std::string, std::function<void()>> caches = {
        //{"geo_ad" , [&geo_ad_cache](){geo_ad_cache.load();}},
        {"ico_ref", [&ico_ref_cache]   (){ico_ref_cache.load();}   },
        {"ad"     , [&ad_cache]    (){ad_cache.load();}    },
        {""       , [&bidder_caches]    (){bidder_caches.load();}    }
    };
   
****/ 
    //initialize and setup CRUD dispatcher
    restful_dispatcher_t dispatcher(config.get("ico-cache-loader.root")) ;
    dispatcher.crud_match(boost::regex("/ico_cache_loader/(\\w*)"))
              .put([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
              LOG(info) << "received cache update event url=" << match[0];
              try {
                  //caches[match[1]]();
              } catch (std::exception const& e) {
                  LOG(error) << e.what();
              }
    });
    auto host = config.get("ico-cache-loader.host");
    auto port = config.get("ico-cache-loader.port");
    http::server::server<restful_dispatcher_t> server(host,port,dispatcher);
    server.run();
}


