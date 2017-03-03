
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include "exchange/exchange_handler.hpp"
#include "exchange/exchange_server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "DSL/generic_dsl.hpp"
#include "rtb/config/config.hpp"
#include "core/tagged_tuple.hpp"
#include "datacache/ad_entity.hpp"
#include "datacache/geo_ad_entity.hpp"
#include "datacache/city_country_entity.hpp"
#include "datacache/entity_cache.hpp"
#include "datacache/memory_types.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <vector>
#include <random>
#include "rtb/common/perf_timer.hpp"
#include "config.hpp"


#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;


int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    
    CacheLoadConfig config([](cache_loader_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
//            ("bidder.log", boost::program_options::value<std::string>(&d.log_file_name), "cache_load_test log file name log")
//            ("bidder.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("data/ads"), "ads_source file name")
//            ("bidder.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
//            ("bidder.geo_ad_source", boost::program_options::value<std::string>(&d.geo_ad_source)->default_value("data/ad_geo"), "geo_ad_source file name")
//            ("bidder.geo_ad_ipc_name", boost::program_options::value<std::string>(&d.geo_ad_ipc_name)->default_value("vanilla-geo-ad-ipc"), "geo ad-ipc name")
//            ("bidder.geo_source", boost::program_options::value<std::string>(&d.geo_source)->default_value("data/geo"), "geo_source file name")
//            ("bidder.geo_ipc_name", boost::program_options::value<std::string>(&d.geo_ipc_name)->default_value("vanilla-geo-ipc"), "geo ipc name")
            ("cache-loader.host", "cache_loader_test Host")
            ("cache-loader.port", "cache_loader_test Port")
            ("cache-loader.root", "cache_loader_test Root")
            ("cache-loader.concurrency", boost::program_options::value<unsigned int>(&d.concurrency)->default_value(0), "cache-loader concurrency, if 0 is set std::thread::hardware_concurrency()")
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
    
    connection_endpoint ep {std::make_tuple(config.get("cache-loader.host"), config.get("cache-loader.port"), config.get("cache-loader.root"))};

    //initialize and setup CRUD dispatchers
    restful_dispatcher_t dispatcher(ep.root) ;
    dispatcher.crud_match(boost::regex("/cache_loader/(\\w+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                 // bid_handler.handle_post(r,match);
                 // pick bidder from config and get cache names from that bidder ??
              });

    LOG(debug) << "concurrency " << config.data().concurrency;
    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.set_concurrency(config.data().concurrency).run() ;
}


