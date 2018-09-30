
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/program_options.hpp>
#include "exchange/exchange_handler.hpp"
#include "exchange/exchange_server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "rtb/DSL/generic_dsl.hpp"
#include "rtb/DSL/any_mapper.hpp"
#include "rtb/DSL/rapid_mapper.hpp"
#include "rtb/config/config.hpp"
#include "rtb/messaging/serialization.hpp"
#include "rtb/messaging/communicator.hpp"
#include "jsonv/all.hpp"

#include "rtb/core/core.hpp"

extern void init_framework_logging(const std::string &) ;

struct exchange_config_data {
    std::string log_file_name;
    int handler_timeout_v1;
    int handler_timeout_v2;
    
    exchange_config_data() :
        log_file_name{}, handler_timeout_v1{}, handler_timeout_v2{}
    {}
};

using namespace vanilla::messaging;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using DSLT  = DSL::GenericDSL<jsonv::string_view> ;
    using DSLTA = DSL::GenericDSL<jsonv::string_view, DSL::any_mapper> ;
    using DSLTR = DSL::GenericDSL<jsonv::string_view, DSL::rapid_mapper> ;
    using BidRequest = DSLT::deserialized_type;
    using BidResponse = DSLT::serialized_type;
   
    int n_bid{};
    unsigned short port{};
    vanilla::config::config<exchange_config_data> config([&](exchange_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("exchange.log", boost::program_options::value<std::string>(&d.log_file_name), "exchange_handler_test log file name log")
            ("exchange.host", "exchange_handler_test Host")
            ("exchange.port", "exchange_handler_test Port")
            ("exchange.root", "exchange_handler_test Root")
            ("exchange.v1.timeout", boost::program_options::value<int>(&d.handler_timeout_v1), "exchange_handler_test v1 timeout")
            ("exchange.v2.timeout", boost::program_options::value<int>(&d.handler_timeout_v2), "exchange_handler_test v2 timeout")
            ("mock-bidder.port", po::value<unsigned short>(&port)->default_value(5000), "udp port for broadcast")
            ("mock-bidder.num_of_bidders", po::value<int>(&n_bid)->default_value(1), "number of bidders to wait for")
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
   
    // ab.sh -n30000 -c10 --auction 
    exchange_handler<DSLT> openrtb_handler(std::chrono::milliseconds(config.data().handler_timeout_v1));
    openrtb_handler    
    .logger([](const std::string &data) {
//        LOG(debug) << "request_data_v1=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request v1 error " << data ;
    })
    .if_response([](const auto &bid_response) {
        using http::server::reply;
        return [code = bid_response.seatbid.empty() ? reply::no_content : reply::not_implemented]
        (http::server::reply &r) {
            r = http::server::reply::stock_reply(code);
        };
    })
    .auction_async([](const auto &request) {
        //TODO: send to the auction Asynchronously with timeout or bid directly in this handler
        return  BidResponse();
    });
    
    // ab.sh -n30000 -c10 --auction-any 
    exchange_handler<DSLTA> openrtb_handler_any(std::chrono::milliseconds(config.data().handler_timeout_v1));
    openrtb_handler_any
    .logger([](const std::string &data) {
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request v1 error " << data ;
    })
    .if_response([](auto&& bid_response) {
        using http::server::reply;
        return [code = bid_response.seatbid.empty() ? reply::no_content : reply::not_implemented]
        (http::server::reply &r) {
            r = http::server::reply::stock_reply(code);
        };
    })
    .auction_async([](const auto &request) {
        return  BidResponse();
    });
    
    // ab.sh -n30000 -c10 --auction-rapid
    exchange_handler<DSLTR> openrtb_handler_rapid(std::chrono::milliseconds(config.data().handler_timeout_v1));
    openrtb_handler_rapid
    .logger([](const std::string &data) {
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request v1 error " << data ;
    })
    .if_response([](const auto &bid_response) {
        using http::server::reply;
        return [code = bid_response.seatbid.empty() ? reply::no_content : reply::not_implemented]
        (http::server::reply &r) {
            r = http::server::reply::stock_reply(code);
        };
    })
    .auction_async([](const auto &request) {
        return  BidResponse();
    });

    //you can put as many exchange handlers as unique URI
    exchange_handler<DSLT> openrtb_handler_v2(std::chrono::milliseconds(config.data().handler_timeout_v2));
    openrtb_handler_v2
    .logger([](const std::string &data) {
        LOG(debug) << "request_data_v2=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request v2 error " << data ;
    })
    .auction_async([&uuid_generator](const BidRequest &request) {
        //TODO: send to the auction synchronously with timeout or bid directly in this handler
        BidResponse response;
        boost::uuids::uuid id = uuid_generator() ;
        thread_local std::string id_str = boost::uuids::to_string(id);
        response.bidid = id_str;
        return response;
    });

    // or you can broadcast to your farm of multiple bidders on multiple remote machines
    exchange_handler<DSLT> openrtb_handler_distributor(std::chrono::milliseconds(config.data().handler_timeout_v2));
    openrtb_handler_distributor
    .logger([](const std::string &data) {
        //LOG(debug) << "request_data for distribution=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request for distribution error " << data ;
    })
    .auction_async([n_bid,port](const BidRequest &request) {
        std::vector<BidResponse> responses;
        communicator<broadcast>()
        .outbound(port)
        .distribute(request)
        .collect<BidResponse>(10ms, [&responses,n_bid](BidResponse bid, auto done) { //move ctored by collect()
            responses.push_back(bid);
            if ( responses.size() == n_bid) {
                done();
            }
        });
        return responses.empty() ? BidResponse() : responses[0];
    });

    connection_endpoint ep {std::make_tuple(config.get("exchange.host"), config.get("exchange.port"), config.get("exchange.root"))};

    //initialize and setup CRUD dispatchers
    restful_dispatcher_t dispatcher(ep.root) ;
    dispatcher.crud_match(boost::regex("/openrtb_handler/auction/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  openrtb_handler.handle_post(r,match);
              });
    dispatcher.crud_match(boost::regex("/openrtb_handler/auction-any/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  openrtb_handler_any.handle_post(r,match);
              });
    dispatcher.crud_match(boost::regex("/openrtb_handler/auction-rapid/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  openrtb_handler_rapid.handle_post(r,match);
              });
    dispatcher.crud_match(boost::regex("/openrtb_handler/(v2[.][2-4])/auction/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  openrtb_handler_v2.handle_post(r,match);
              });
    dispatcher.crud_match(boost::regex("/openrtb_handler/mock-bidders/auction/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  openrtb_handler_distributor.handle_post(r,match);
              });

    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.run() ;
}


