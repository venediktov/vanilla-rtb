/* 
 * File:   campaign_cache.hpp
 * Author: vladimir venediktov
 *
 * Created on March 12, 2017, 10:22 PM
 */

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include "CRUD/service/server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "rtb/config/config.hpp"
#include "rtb/common/perf_timer.hpp"
#include "config.hpp"
#include "campaign_cache.hpp"


#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;


int main(int argc, char *argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    std::string create_restful_prefix;
    std::string update_restful_prefix;
    std::string delete_restful_prefix;
 
    CampaignManagerConfig config([](campaign_manager_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("campaign-manager.log", boost::program_options::value<std::string>(&d.log_file_name), "campaign_manager_test log file name log")
            ("campaign-manager.host", "campaign_manager_test Host")
            ("campaign-manager.port", "campaign_manager_test Port")
            ("campaign-manager.root", "campaign_manager_test Root")
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

//    vanilla::Selector<CacheLoadConfig> selector(config);
    CampaignCache<CampaignManagerConfig>  campaign_manager_cache(config);
//    GeoDataEntity<CacheLoadConfig>    geo_cache(config);
//    AdDataEntity<CacheLoadConfig>     ad_cache(config);
//    
//    std::map<std::string, std::function<void()>> create_commands = {
//        {"campaign" , [&geo_ad_cache](){geo_ad_cache.load();}},
//        {"geo"    , [&geo_cache]   (){geo_cache.load();}   },
//        {"ad"     , [&ad_cache]    (){ad_cache.load();}    },
//        {""       , [&selector]    (){selector.load();}    }
//    };
    
    //initialize and setup CRUD dispatcher
    restful_dispatcher_t dispatcher(config.get("campaign-manager.root")) ;
    dispatcher.crud_match(boost::regex("/campaign_manager/(\\w*)"))
              .put([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
              LOG(info) << "received cache update event url=" << match[0];
              try {
                  //create_commands[match[1]]();
              } catch (std::exception const& e) {
                  LOG(error) << e.what();
              }
    });
    auto host = config.get("campaign-manager.host");
    auto port = config.get("campaign-manager.port");
    http::server::server<restful_dispatcher_t> server(host,port,dispatcher);
    server.run();
}


