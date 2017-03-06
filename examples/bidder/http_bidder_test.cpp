
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

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

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
    
    vanilla::Selector<> selector(config);
    boost::uuids::random_generator uuid_generator{};
    
    try {
        selector.load();
    }
    catch(std::exception const& e) {
        LOG(error) << e.what();
        return 0;
    }
    exchange_handler<DSL::GenericDSL> bid_handler(std::chrono::milliseconds(config.data().timeout));
    bid_handler    
        .logger([](const std::string &data) {
            //LOG(debug) << "bid request=" << data ;
        })
        .error_logger([](const std::string &data) {
            LOG(debug) << "bid request error " << data ;
        })
        .auction([&](const openrtb::BidRequest &request) {
            openrtb::BidResponse response;
            for(auto &imp : request.imp) {    
                if(auto ad = selector.getAd(request, imp)) {
                    auto sp = std::make_shared<std::stringstream>();
                    {
                        perf_timer<std::stringstream> timer(sp, "fill response");
                        
                        boost::uuids::uuid bidid = uuid_generator();
                        response.bidid = boost::uuids::to_string(bidid);

                        if (request.cur.size()) {
                            response.cur = request.cur[0];
                        } else if (imp.bidfloorcur.length()) {
                            response.cur = imp.bidfloorcur; // Just return back
                        }

                        if (response.seatbid.size() == 0) {
                            response.seatbid.emplace_back(openrtb::SeatBid());
                        }

                        openrtb::Bid bid;
                        bid.id = boost::uuids::to_string(bidid); // TODO check documentation 
                        // Is it the same as response.bidid?
                        bid.impid = imp.id;
                        bid.price = ad->max_bid_micros / 1000000.0; // Not micros?
                        bid.w = ad->width;
                        bid.h = ad->height;
                        bid.adm = ad->code;
                        bid.adid = ad->ad_id;

                        response.seatbid.back().bid.emplace_back(std::move(bid));
                    }
                    LOG(debug) << sp->str();
                }
            }
            
            
            return response;
        });
    
    connection_endpoint ep {std::make_tuple(config.get("bidder.host"), config.get("bidder.port"), config.get("bidder.root"))};

    //initialize and setup CRUD dispatchers
    restful_dispatcher_t dispatcher(ep.root) ;
    dispatcher.crud_match(boost::regex("/bid/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  bid_handler.handle_post(r,match);
              });

    LOG(debug) << "concurrency " << config.data().concurrency;
    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.set_concurrency(config.data().concurrency).run() ;
}


