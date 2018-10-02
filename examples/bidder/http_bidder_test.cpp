#include <vector>
#include <random>
#include <utility>
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "rtb/core/core.hpp"
#include "rtb/exchange/exchange_handler.hpp"
#include "rtb/exchange/exchange_server.hpp"
#include "rtb/DSL/generic_dsl.hpp"
#include "rtb/DSL/any_mapper.hpp"
#include "rtb/DSL/rapid_mapper.hpp"
#include "rtb/config/config.hpp"
#include "rtb/core/tagged_tuple.hpp"
#include "rtb/datacache/entity_cache.hpp"
#include "rtb/datacache/memory_types.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "examples/datacache/city_country_entity.hpp"
#include "bidder.hpp"

#include "serialization.hpp"
#include "rtb/common/decision_router.hpp"
#include "rtb/client/empty_key_value_client.hpp"

extern void init_framework_logging(const std::string &) ;

//auto random_pick(int max) {
//  std::random_device rd;
//  std::mt19937 gen(rd());
//  std::uniform_int_distribution<> dis(1, max);
//  return dis(gen);
//}

namespace bidder_decision_codes {
        enum {EXIT=-1, USER_DATA=0, NO_BID, AUCTION_ASYNC, SIZE};
}

int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using DSLT = DSL::GenericDSL<std::string, DSL::rapid_mapper> ;
    using BidRequest = DSLT::deserialized_type;
    //using BidResponse = DSLT::serialized_type;
    
    BidderConfig config([](bidder_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("bidder.log", boost::program_options::value<std::string>(&d.log_file_name), "bidder_test log file name log")
            ("bidder.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("data/ads"), "ads_source file name")
            ("bidder.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
            ("bidder.geo_source", boost::program_options::value<std::string>(&d.geo_source)->default_value("data/geo"), "geo_source file name")
            ("bidder.geo_ipc_name", boost::program_options::value<std::string>(&d.geo_ipc_name)->default_value("vanilla-geo-ipc"), "geo ipc name")
            ("bidder.port", boost::program_options::value<short>(&d.port)->required(), "bidder port")
            ("bidder.host", boost::program_options::value<std::string>(&d.host)->default_value("0.0.0.0"), "bidder host")
            ("bidder.root", boost::program_options::value<std::string>(&d.root)->default_value("."), "bidder root")
            ("bidder.timeout", boost::program_options::value<int>(&d.timeout), "bidder_test timeout")
            ("bidder.concurrency", boost::program_options::value<unsigned int>(&d.concurrency)->default_value(0), "bidder concurrency, if 0 is set std::thread::hardware_concurrency()")
            ("bidder.geo_campaign_ipc_name", boost::program_options::value<std::string>(&d.geo_campaign_ipc_name)->default_value("vanilla-geo-campaign-ipc"), "geo campaign ipc name")
            ("bidder.geo_campaign_source", boost::program_options::value<std::string>(&d.geo_campaign_source)->default_value("data/geo_campaign"), "geo_campaign_source file name")
            ("campaign-manager.ipc_name", boost::program_options::value<std::string>(&d.ipc_name)->default_value("vanilla-campaign-budget-ipc"),"campaign_budget IPC name")
            ("campaign-manager.budget_source", boost::program_options::value<std::string>(&d.campaign_budget_source)->default_value("data/campaign_budget"),"campaign_budget source file name")
            ("bidder.key_value_host", boost::program_options::value<std::string>(&d.key_value_host)->default_value("0.0.0.0"), "key value storage host")
            ("bidder.key_value_port", boost::program_options::value<int>(&d.key_value_port)->default_value(0), "key value storage port")
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
    
    boost::uuids::random_generator uuid_generator{};
    vanilla::BidderCaches<> caches(config);
    try {
        caches.load(); // Not needed if data cache loader is in work
    }
    catch(std::exception const& e) {
        LOG(error) << e.what();
        return 0;
    }
    
    using bid_handler_type = exchange_handler<DSLT, vanilla::UserInfo>;   
    using decision_router_type = vanilla::decision::router < bidder_decision_codes::SIZE , 
                                                                 http::server::reply& , 
                                                                 BidRequest& ,
                                                                 vanilla::UserInfo& 
                                                               >;
    
    bid_handler_type bid_handler(std::chrono::milliseconds(config.data().timeout));
    
    auto request_user_data_f = [&bid_handler, &config](http::server::reply &reply, BidRequest &, auto && info) {
        using kv_type = vanilla::client::empty_key_value_client;
        thread_local kv_type kv_client;
        bool is_matched_user = info.user_id.length();
        if (!is_matched_user) {
            return true; // bid unmatched
        }
        if (!kv_client.connected()) {
            kv_client.connect(config.data().key_value_host, config.data().key_value_port);
        }
        kv_client.request(info.user_id, info.user_data);
        return true;
    };
    auto no_bid_f = [&bid_handler, &config](http::server::reply &reply, BidRequest &, auto&&) {
        reply << http::server::reply::flush("");
        return true;
    };
    auto auction_async_f = [&bid_handler](http::server::reply &reply, BidRequest & bid_request, auto&&) {
        return bid_handler.handle_auction_async(reply, bid_request);
    };
    const decision_router_type::decision_tree_type decision_tree = {{
        {bidder_decision_codes::USER_DATA, {request_user_data_f, bidder_decision_codes::AUCTION_ASYNC, bidder_decision_codes::NO_BID}},
        {bidder_decision_codes::NO_BID, {no_bid_f, bidder_decision_codes::EXIT, bidder_decision_codes::EXIT}},        
        {bidder_decision_codes::AUCTION_ASYNC, {auction_async_f, bidder_decision_codes::EXIT, bidder_decision_codes::EXIT}}
    }};
    decision_router_type decision_router(decision_tree);
    
    bid_handler    
        .logger([](const std::string &data) {
            //LOG(debug) << "bid request=" << data ;
        })
        .error_logger([](const std::string &data) {
            LOG(debug) << "bid request error " << data ;
        })
        .auction_async([&](const BidRequest &request, auto && ...info) {
            thread_local vanilla::Bidder<DSLT, BidderConfig> bidder(caches);
            return bidder.bid(request, info...);
        })
        .decision([&decision_router](auto && ... args) {
            vanilla::UserInfo info;
            decision_router.execute(args... , info);
        });
    
    connection_endpoint ep {std::make_tuple(config.data().host, boost::lexical_cast<std::string>(config.data().port), config.data().root)};

    //initialize and setup CRUD dispatchers
    restful_dispatcher_t dispatcher(ep.root);
    dispatcher.crud_match(boost::regex("/bid/(\\d+)"))
        .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            bid_handler.handle_post(r, match);
        });
    dispatcher.crud_match(boost::regex("/test/"))
        .post([](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            //r << "test";
            //r.stock_reply(http::server::reply::ok);
            r << "test" << http::server::reply::flush("text");
        });

    LOG(debug) << "concurrency " << config.data().concurrency;
    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.set_concurrency(config.data().concurrency).run() ;
}


