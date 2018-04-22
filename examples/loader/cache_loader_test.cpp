
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include "CRUD/service/server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "rtb/config/config.hpp"
#include "datacache/entity_cache.hpp"
#include "datacache/memory_types.hpp"

#include "examples/datacache/city_country_entity.hpp"
#include "examples/matchers/geo_campaign.hpp"
#include "examples/matchers/geo.hpp"


#include "examples/matchers/ico_campaign.hpp"
#include "examples/matchers/domain.hpp"
#include "examples/matchers/ad.hpp"
#include "examples/campaign/campaign_cache.hpp"
#include "rtb/datacache/generic_bidder_cache_loader.hpp"
#include "config.hpp"


extern void init_framework_logging(const std::string &) ;


int main(int argc, char *argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using CacheLoadConfig    = vanilla::config::config<cache_loader_config_data> ;
    using GeoEntityT         = GeoDataEntity<CacheLoadConfig> ;
    using GeoCampaignEntityT = GeoCampaignEntity<CacheLoadConfig> ;
    using AdEntityT          = AdDataEntity<CacheLoadConfig> ;
    using BudgetEntityT      = vanilla::CampaignCache<CacheLoadConfig> ;
    using CacheLoader        =  vanilla::GenericBidderCacheLoader<GeoEntityT, GeoCampaignEntityT, AdEntityT, BudgetEntityT> ;
    
    CacheLoadConfig config([](cache_loader_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("cache-loader.log", boost::program_options::value<std::string>(&d.log_file_name), "cache_loader_test log file name log")
            ("cache-loader.host", "cache_loader_test Host")
            ("cache-loader.port", "cache_loader_test Port")
            ("cache-loader.root", "cache_loader_test Root")
            ("datacache.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("data/ads"), "ads_source file name")
            ("datacache.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
            ("datacache.geo_capmaign_source", boost::program_options::value<std::string>(&d.geo_campaign_source)->default_value("data/geo_campaign"), "geo_ad_source file name")
            ("datacache.geo_campaign_ipc_name", boost::program_options::value<std::string>(&d.geo_campaign_ipc_name)->default_value("vanilla-geo-campaign-ipc"), "geo campaign ipc name")
            ("datacache.geo_source", boost::program_options::value<std::string>(&d.geo_source)->default_value("data/geo"), "geo_source file name")
            ("datacache.geo_ipc_name", boost::program_options::value<std::string>(&d.geo_ipc_name)->default_value("vanilla-geo-ipc"), "geo ipc name")        
            ("bidder.geo_campaign_ipc_name", boost::program_options::value<std::string>(&d.geo_campaign_ipc_name)->default_value("vanilla-geo-campaign-ipc"), "geo campaign ipc name")
            ("bidder.geo_campaign_source", boost::program_options::value<std::string>(&d.geo_campaign_source)->default_value("data/geo_campaign"), "geo_campaign_source file name")
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
    CacheLoader bidder_caches(config);

    std::map<std::string, std::function<void()>> caches = {
        {"geo_campaign" , [&bidder_caches](){bidder_caches.get<GeoCampaignEntityT>().load();  }},
        {"geo"    , [&bidder_caches](){bidder_caches.get<GeoEntityT>().load();    }},
        {"ad"     , [&bidder_caches](){bidder_caches.get<AdEntityT>().load();     }},
        {"budget",  [&bidder_caches](){bidder_caches.get<BudgetEntityT>().load(); }},
        {""       , [&bidder_caches](){bidder_caches.load();   }}
    };
    
    //initialize and setup CRUD dispatcher
    restful_dispatcher_t dispatcher(config.get("cache-loader.root")) ;
    dispatcher.crud_match(boost::regex("/cache_loader/(\\w*)"))
              .put([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
              LOG(info) << "received cache update event url=" << match[0];
              try {
                  caches[match[1]]();
              } catch (std::exception const& e) {
                  LOG(error) << e.what();
              }
    });
    auto host = config.get("cache-loader.host");
    auto port = config.get("cache-loader.port");
    http::server::server<restful_dispatcher_t> server(host,port,dispatcher);
    server.run();
}


