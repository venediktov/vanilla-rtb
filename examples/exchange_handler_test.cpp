
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

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

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

int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
   
    
    vanilla::config::config<exchange_config_data> config([](exchange_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("exchange.log", boost::program_options::value<std::string>(&d.log_file_name), "exchange_handler_test log file name log")
            ("exchange.host", "exchange_handler_test Host")
            ("exchange.port", "exchange_handler_test Port")
            ("exchange.root", "exchange_handler_test Root")
            ("exchange.v1.timeout", boost::program_options::value<int>(&d.handler_timeout_v1), "exchange_handler_test v1 timeout")
            ("exchange.v2.timeout", boost::program_options::value<int>(&d.handler_timeout_v2), "exchange_handler_test v2 timeout")
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
    
    exchange_handler<DSL::GenericDSL> openrtb_handler(std::chrono::milliseconds(config.data().handler_timeout_v1));
    openrtb_handler    
    .logger([](const std::string &data) {
        LOG(debug) << "request_data_v1=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request v1 error " << data ;
    })
    .auction([](const openrtb::BidRequest &request) {
        //TODO: send to the auction synchronously with timeout or bid directly in this handler
        openrtb::BidResponse response;
        return response;
    });

    //you can put as many exchange handlers as unique URI
    exchange_handler<DSL::GenericDSL> openrtb_handler_v2(std::chrono::milliseconds(config.data().handler_timeout_v2));
    openrtb_handler_v2
    .logger([](const std::string &data) {
        LOG(debug) << "request_data_v2=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request v2 error " << data ;
    })
    .auction([&uuid_generator](const openrtb::BidRequest &request) {
        //TODO: send to the auction synchronously with timeout or bid directly in this handler
        openrtb::BidResponse response;
        boost::uuids::uuid id = uuid_generator() ; 
        response.bidid = boost::uuids::to_string(id);
        return response;
    });

    // or you can broadcast to your farm of multiple bidders on multiple remote machines
    exchange_handler<DSL::GenericDSL> openrtb_handler_distributor(std::chrono::milliseconds(config.data().handler_timeout_v2));
    openrtb_handler_distributor
    .logger([](const std::string &data) {
        LOG(debug) << "request_data for distribution=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request for distribution error " << data ;
    })
    .auction([](const openrtb::BidRequest &request) {
        std::vector<openrtb::BidResponse> responses;
        communicator<broadcast>()
        .outbound(5000)
        .distribute(openrtb::BidRequest())
        .collect<openrtb::BidResponse>(10ms, [&responses](openrtb::BidResponse bid, auto done) { //move ctored by collect()
            responses.push_back(bid);
        });
        return responses.empty() ? openrtb::BidResponse() : responses[0];
    });

    connection_endpoint ep {std::make_tuple(config.get("exchange.host"), config.get("exchange.port"), config.get("exchange.root"))};

    //initialize and setup CRUD dispatchers
    restful_dispatcher_t dispatcher(ep.root) ;
    dispatcher.crud_match(boost::regex("/openrtb_handler/auction/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  openrtb_handler.handle_post(r,match);
              });
    dispatcher.crud_match(boost::regex("/openrtb_handler/(v2[.][2-4])/auction/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  openrtb_handler_v2.handle_post(r,match);
              });

    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.run() ;
}


