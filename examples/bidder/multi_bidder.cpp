
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
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
#include "selector.hpp"
#include "serialization.hpp"

#include "rtb/messaging/communicator.hpp"
#include "rtb/messaging/serialization.hpp"
#if !defined(WIN32)
#include <unistd.h>
#include "rtb/core/process.hpp"
#else
#include <process.h>
#endif
#include "reponse_builder.hpp"

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

void run(short port, const BidderConfig &config) {
    using namespace vanilla::messaging;
    vanilla::ResponseBuilder<BidderConfig> response_builder(config);
    communicator<broadcast>().inbound(port).process<openrtb::BidRequest>([&response_builder](auto endpoint, openrtb::BidRequest request) {
        return response_builder.build(request);
    }).dispatch();
}

int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    namespace po = boost::program_options;   
    
    BidderConfig config([](bidder_config_data &d, po::options_description &desc){
        desc.add_options()
            ("multi_bidder.log", po::value<std::string>(&d.log_file_name), "bidder_test log file name log")
            ("multi_bidder.ads_source", po::value<std::string>(&d.ads_source)->default_value("data/ads"), "ads_source file name")
            ("multi_bidder.ads_ipc_name", po::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
            ("multi_bidder.geo_ad_source", po::value<std::string>(&d.geo_ad_source)->default_value("data/ad_geo"), "geo_ad_source file name")
            ("multi_bidder.geo_ad_ipc_name", po::value<std::string>(&d.geo_ad_ipc_name)->default_value("vanilla-geo-ad-ipc"), "geo ad-ipc name")
            ("multi_bidder.geo_source", po::value<std::string>(&d.geo_source)->default_value("data/geo"), "geo_source file name")
            ("multi_bidder.geo_ipc_name", po::value<std::string>(&d.geo_ipc_name)->default_value("vanilla-geo-ipc"), "geo ipc name")
            ("multi_bidder.host", "bidder_test Host")
            ("multi_bidder.port", po::value<short>(&d.port)->required(), "bidder_dest Port")
            ("multi_bidder.root", "bidder_test Root")
            ("multi_bidder.timeout", po::value<int>(&d.timeout), "bidder_test timeout")
            ("multi_bidder.concurrency", po::value<unsigned int>(&d.concurrency)->default_value(0), "bidder concurrency, if 0 is set std::thread::hardware_concurrency()")
            ("multi_bidder.num_of_bidders", po::value<short>(&d.num_of_bidders)->default_value(1), "number of bidders")
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
    
    // TODO load should be made in datacache loader
    vanilla::Selector<> selector(config);
    try {
        selector.load();
    }
    catch(std::exception const& e) {
        LOG(error) << e.what();
        return 0;
    }
    if(1 == config.data().num_of_bidders) {
        run(config.data().port, config);
    }
#if !defined(WIN32)
    else {
        using OS::UNIX::Process;
        try {
            auto handle = [&config](unsigned int port) {
                LOG(info) << "Starting mock bidder pid=" << getpid();
                run(config.data().port, config);
            };
            using Handler = decltype(handle);
            Process<> parent_proc;
            Process<Handler> child_proc(handle);
            std::vector<decltype(child_proc) > child_procs(config.data().num_of_bidders, child_proc);
            auto spawned_procs = parent_proc.spawn(child_procs, config.data().port);
            parent_proc.wait(spawned_procs);
        } catch (const std::exception &e) {
            LOG(error) << e.what();
            return (EXIT_FAILURE);
        }
    }
#endif
    LOG(debug) << "exit";
    return (EXIT_SUCCESS);
}


