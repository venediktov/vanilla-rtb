#include <vector>
#include <random>
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "rtb/core/core.hpp"
#include "rtb/exchange/exchange_handler.hpp"
#include "rtb/exchange/exchange_server.hpp"
#include "rtb/DSL/generic_dsl.hpp"
#include "rtb/config/config.hpp"
#include "rtb/core/tagged_tuple.hpp"
#include "rtb/datacache/ad_entity.hpp"
#include "rtb/datacache/entity_cache.hpp"
#include "rtb/datacache/memory_types.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "examples/datacache/geo_entity.hpp"
#include "examples/datacache/city_country_entity.hpp"

#include "rtb/common/perf_timer.hpp"
#include "config.hpp"
#include "serialization.hpp"
#include "bidder_selector.hpp"


extern void init_framework_logging(const std::string &) ;




auto random_pick(int max) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, max);
  return dis(gen);
}


int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using BidRequest = openrtb::BidRequest<std::string>;
    using BidResponse = openrtb::BidResponse<std::string>;
    using SeatBid = openrtb::SeatBid<std::string>;
    using Bid = openrtb::Bid<std::string>;
    
    BidderConfig config([](bidder_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("bidder.log", boost::program_options::value<std::string>(&d.log_file_name), "bidder_test log file name log")
            ("bidder.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("data/ads"), "ads_source file name")
            ("bidder.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
            ("bidder.geo_ad_source", boost::program_options::value<std::string>(&d.geo_ad_source)->default_value("data/ad_geo"), "geo_ad_source file name")
            ("bidder.geo_ad_ipc_name", boost::program_options::value<std::string>(&d.geo_ad_ipc_name)->default_value("vanilla-geo-ad-ipc"), "geo ad-ipc name")
            ("bidder.geo_source", boost::program_options::value<std::string>(&d.geo_source)->default_value("data/geo"), "geo_source file name")
            ("bidder.geo_ipc_name", boost::program_options::value<std::string>(&d.geo_ipc_name)->default_value("vanilla-geo-ipc"), "geo ipc name")
            ("bidder.host", "bidder_test Host")
            ("bidder.port", "bidder_est Port")
            ("bidder.root", "bidder_test Root")
            ("bidder.timeout", boost::program_options::value<int>(&d.timeout), "bidder_test timeout")
            ("bidder.concurrency", boost::program_options::value<unsigned int>(&d.concurrency)->default_value(0), "bidder concurrency, if 0 is set std::thread::hardware_concurrency()")
            ("bidder.geo_campaign_ipc_name", boost::program_options::value<std::string>(&d.geo_campaign_ipc_name)->default_value("vanilla-geo-campaign-ipc"), "geo campaign ipc name")
            ("bidder.geo_campaign_source", boost::program_options::value<std::string>(&d.geo_campaign_source)->default_value("data/geo_campaign"), "geo_campaign_source file name")
            ("bidder.campaign_data_ipc_name", boost::program_options::value<std::string>(&d.campaign_data_ipc_name)->default_value("vanilla-campaign-data-ipc"), "campaign data ipc name")
            ("bidder.campaign_data_source", boost::program_options::value<std::string>(&d.campaign_data_source)->default_value("data/campaign_data"), "campaign_data_source file name")
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
    
    //vanilla::Selector<> selector(config); 
    boost::uuids::random_generator uuid_generator{};
    vanilla::BidderCaches<> caches(config);
    try {
        caches.load(); // Not needed if data cache loader is in work
        //selector.load();
    }
    catch(std::exception const& e) {
        LOG(error) << e.what();
        return 0;
    }
    exchange_handler<DSL::GenericDSL<>> bid_handler(std::chrono::milliseconds(config.data().timeout));
    bid_handler    
        .logger([](const std::string &data) {
            //LOG(debug) << "bid request=" << data ;
        })
        .error_logger([](const std::string &data) {
            LOG(debug) << "bid request error " << data ;
        })

        .auction_async([&](const BidRequest &request) {

            thread_local vanilla::BidderSelector<> selector(caches);
            BidResponse response;
            for(auto &imp : request.imp) {
                if(auto ad = selector.select(request, imp)) {
                   boost::uuids::uuid bidid = uuid_generator();
                   response.bidid = boost::uuids::to_string(bidid);
                   if (request.cur.size()) {
                       response.cur = request.cur[0];
                   } else if (imp.bidfloorcur.length()) {
                       response.cur = imp.bidfloorcur; // Just return back
                   }
                   Bid bid;
                   bid.id = boost::uuids::to_string(bidid); // TODO check documentation 
                   // Is it the same as response.bidid?
                   // Wrong filling type
                   bid.impid = imp.id;
                   bid.price = ad->max_bid_micros / 1000000.0; // Not micros?
                   bid.w = ad->width;
                   bid.h = ad->height;
                   bid.adm = ad->code;
                   bid.adid = ad->ad_id;
                   if (response.seatbid.size() == 0) {
                      SeatBid seatbid;
                      seatbid.bid.push_back(bid);
                      response.seatbid.push_back(seatbid);
                   } else {
                      response.seatbid.back().bid.push_back(bid);
                   }
                }
            }
            return response;
        });
    
    connection_endpoint ep {std::make_tuple(config.get("bidder.host"), config.get("bidder.port"), config.get("bidder.root"))};

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


