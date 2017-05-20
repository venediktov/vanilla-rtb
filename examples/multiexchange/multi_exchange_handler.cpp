/* 
 * File:   multi_exchange_handler.cpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 27 февраля 2017 г., 23:33
 */

#include <chrono>
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/program_options.hpp>
#include "exchange/exchange_handler.hpp"
#include "exchange/exchange_server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "rtb/DSL/generic_dsl.hpp"
#include "rtb/config/config.hpp"
#include "rtb/messaging/serialization.hpp"
#include "rtb/messaging/communicator.hpp"
#include "core/tagged_tuple.hpp"
#include <boost/memory_order.hpp>
#include "user_info.hpp"
 

#include <boost/asio/ip/address.hpp>

#include "multiexchange_config.hpp"
#include "multiexchange_status.hpp"
#include "rtb/exchange/multibidder_communicator.hpp"
#include "rtb/client/empty_key_value_client.hpp"

#include "rtb/core/core.hpp"

extern void init_framework_logging(const std::string &) ;

int main(int argc, char* argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using namespace vanilla::exchange;
    using namespace vanilla::multiexchange;
    namespace po = boost::program_options;   
 
    using string_view = std::string;
    using BidRequest = openrtb::BidRequest<string_view>;
    using BidResponse = openrtb::BidResponse<string_view>;

    vanilla::multiexchange::multiexchange_config config([&](multi_exchange_handler_config_data &d, po::options_description &desc){
        desc.add_options()
            ("multi_exchange.log", po::value<std::string>(&d.log_file_name), "exchange_handler_test log file name log")
            ("multi_exchange.host", "multi_exchange_handler_test Host")
            ("multi_exchange.port", "multi_exchange_handler_test Port")
            ("multi_exchange.root", "multi_exchange_handler_test Root")
            ("multi_bidder.concurrency", po::value<int>(&d.concurrency)->default_value(0), "concurrency")
            ("multi_exchange.timeout", po::value<int>(&d.handler_timeout)->required(), "multi_exchange_handler_timeout")
            ("multi_bidder.timeout", po::value<int>(&d.bidders_response_timeout)->required(), "multi exchange handler bidders request timeout")
            ("multi_bidder.port", po::value<int>(&d.bidders_port)->required(), "udp port for broadcast")
            ("multi_bidder.num_of_bidders", po::value<int>(&d.num_bidders)->default_value(1), "number of bidders to wait for")
            ("multi_bidder.key_value_host", po::value<std::string>(&d.key_value_host), "key value storage host")
            ("multi_bidder.key_value_port", po::value<int>(&d.key_value_port), "key value storage port")
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
    
    // status 
    vanilla::multiexchange::multi_exchange_status status;
    
    // bid exchange handler
    vanilla::exchange::exchange_handler<DSL::GenericDSL<>> openrtb_handler_distributor(std::chrono::milliseconds(config.data().handler_timeout));
    openrtb_handler_distributor
    .logger([](const std::string &data) {
        //LOG(debug) << "request_data for distribution=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request for distribution error " << data ;
    })
    
    .auction_async([&config, &status](const BidRequest &request) {
        using namespace vanilla::messaging;
        ++status.request_count;
                
        vanilla::VanillaRequest vanilla_request;
        vanilla_request.bid_request = request; // optimize
        
        if(request.user) {
            vanilla_request.user_info.user_id = request.user.get().buyeruid;
        }
        vanilla::multibidder_communicator<DSL::GenericDSL<> > communicator(
            config.data().bidders_port, 
            std::chrono::milliseconds(config.data().bidders_response_timeout)
        );
        vanilla::multibidder_collector<string_view> collector(config.data().num_bidders);
        collector
            .clear()
            .on_response([&status](const vanilla::multibidder_collector<string_view>* collector) {
                auto &r = collector->get_reponses();
                if (r.empty()) {
                    ++status.empty_response_count;
                } else if (r.size() == collector->get_num_bidders()) {
                    ++status.all_response_count;
                } else {
                    ++status.timeout_response_count;
                }
            })
            .on_add([&status](const vanilla::multibidder_collector<string_view>* collector) {
                 ++status.bidder_response_count;
            });
        
        
        using kv_type = vanilla::client::empty_key_value_client;
        thread_local kv_type kv_client;
        
        if(!kv_client.connected()) {
            LOG(debug) << "kv not connected";
            kv_client.connect(config.data().key_value_host, config.data().key_value_port);
        }
        bool is_matched_user = vanilla_request.user_info.user_id.length();
        
        if(!is_matched_user || !kv_client.connected()) { // it's not available at all
            LOG(debug) << "KV is not connected yeat";
            communicator.process(vanilla_request, collector);
        }
        else {
            kv_client
                .response([&vanilla_request, &communicator, &collector](){
                    communicator.process(vanilla_request, collector);
                })
                .request(vanilla_request.user_info.user_id, vanilla_request.user_info.user_data);
            
        }
        return collector.response();
        
    });
    
    
    connection_endpoint ep {std::make_tuple(config.get("multi_exchange.host"), config.get("multi_exchange.port"), config.get("multi_exchange.root"))};
    restful_dispatcher_t dispatcher(ep.root) ;
    dispatcher.crud_match(boost::regex("/bid/"))
        .post([&openrtb_handler_distributor](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            openrtb_handler_distributor.handle_post(r,match);
        });
    dispatcher.crud_match(boost::regex("/status.html"))
        .get([&status](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            r << status.to_string();
            r.stock_reply(http::server::reply::ok);
        });

    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    try {
        server.set_concurrency(config.data().concurrency).run() ;
    }
    catch (std::exception const & e) {
        LOG(error) << e.what();
    }
    return 0;
}

