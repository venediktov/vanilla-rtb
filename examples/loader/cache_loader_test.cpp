
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include "CRUD/service/server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "rtb/config/config.hpp"
#include "datacache/ad_entity.hpp"
#include "datacache/geo_ad_entity.hpp"
#include "datacache/city_country_entity.hpp"
#include "datacache/entity_cache.hpp"
#include "datacache/memory_types.hpp"
#include "rtb/common/perf_timer.hpp"
#include "bidder/selector.hpp"
#include "bidder/serialization.hpp"
#include "config.hpp"


#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;


int main(int argc, char *argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    
    CacheLoadConfig config([](cache_loader_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("cache-loader.log", boost::program_options::value<std::string>(&d.log_file_name), "cache_loader_test log file name log")
            ("cache-loader.host", "cache_loader_test Host")
            ("cache-loader.port", "cache_loader_test Port")
            ("cache-loader.root", "cache_loader_test Root")
            ("datacache.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("bidder/data/ads"), "ads_source file name")
            ("datacache.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
            ("datacache.geo_ad_source", boost::program_options::value<std::string>(&d.geo_ad_source)->default_value("bidder/data/ad_geo"), "geo_ad_source file name")
            ("datacache.geo_ad_ipc_name", boost::program_options::value<std::string>(&d.geo_ad_ipc_name)->default_value("vanilla-geo-ad-ipc"), "geo ad-ipc name")
            ("datacache.geo_source", boost::program_options::value<std::string>(&d.geo_source)->default_value("bidder/data/geo"), "geo_source file name")
            ("datacache.geo_ipc_name", boost::program_options::value<std::string>(&d.geo_ipc_name)->default_value("vanilla-geo-ipc"), "geo ipc name")        
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
    vanilla::Selector<CacheLoadConfig> selector(config);
    GeoAdDataEntity<CacheLoadConfig>  geo_ad_cache(config);
    GeoDataEntity<CacheLoadConfig>    geo_cache(config);
    AdDataEntity<CacheLoadConfig>     ad_cache(config);
    
    std::map<std::string, std::function<void()>> caches = {
        {"geo_ad" , [&geo_ad_cache](){geo_ad_cache.load();}},
        {"geo"    , [&geo_cache]   (){geo_cache.load();}   },
        {"ad"     , [&ad_cache]    (){ad_cache.load();}    },
        {""       , [&selector]    (){selector.load();}    }
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


