/* 
 * File:   multi_exchange_handler.cpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 27 февраля 2017 г., 23:33
 */

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
#include <boost/atomic/atomic.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
 

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

struct multi_exchange_handler_config_data {
    std::string log_file_name;
    int handler_timeout;
    int num_bidders;
    int bidders_port;
    int bidders_response_timeout;
    int concurrency;
    
    
    multi_exchange_handler_config_data() :
        log_file_name{}, handler_timeout{}, num_bidders{}, bidders_port{}, bidders_response_timeout{}, concurrency{}
    {}
};

struct multi_exchange_status {
    boost::posix_time::ptime start{boost::posix_time::microsec_clock::local_time()};
    boost::atomic_uint64_t request_count{};
    boost::atomic_uint64_t all_response_count{};
    boost::atomic_uint64_t bidder_response_count{};
    boost::atomic_uint64_t empty_response_count{};
    boost::atomic_uint64_t timeout_response_count{};

    friend std::ostream& operator<<(std::ostream &os, const multi_exchange_status &st) {
        boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - st.start;
        os << "start: " << boost::posix_time::to_simple_string(st.start) << "<br/>" <<
              "elapsed: " << boost::posix_time::to_simple_string(td) << "<br/>" <<
              "<table border=0>" <<
              "<tr><td>requests</td><td>" << st.request_count << "</td></tr>" <<
              "<tr><td>all bidders responsed</td><td>" << st.all_response_count << "</td></tr>" << 
              "<tr><td>bid responses</td><td>" << st.bidder_response_count << "</td></tr>" << 
              "<tr><td>empty responses</td><td>" << st.empty_response_count << "</td></tr>" << 
              "<tr><td>timeout responses</td><td>" << st.timeout_response_count << "</td></tr>" << 
              "</table> ";
        return os;
    }
};
int main(int argc, char* argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using namespace vanilla::exchange;
    namespace po = boost::program_options;   
    vanilla::config::config<multi_exchange_handler_config_data> config([&](multi_exchange_handler_config_data &d, po::options_description &desc){
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
    multi_exchange_status status;
    
    // bid exchange handler
    vanilla::exchange::exchange_handler<DSL::GenericDSL> openrtb_handler_distributor(std::chrono::milliseconds(config.data().handler_timeout));
    openrtb_handler_distributor
    .logger([](const std::string &data) {
        //LOG(debug) << "request_data for distribution=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request for distribution error " << data ;
    })
    .auction([&config, &status](const openrtb::BidRequest &request) {
        using namespace vanilla::messaging;
        ++status.request_count;
        std::vector<openrtb::BidResponse> responses;
        communicator<broadcast>()
        .outbound(config.data().bidders_port)
        .distribute(request)
        .collect<openrtb::BidResponse>(std::chrono::milliseconds(config.data().bidders_response_timeout), [&responses, &config, &status](openrtb::BidResponse bid, auto done) { //move ctored by collect()    
            responses.emplace_back(std::move(bid));
            ++status.bidder_response_count;
            if ( responses.size() == config.data().num_bidders) {
                done();
            }
        });
        if(responses.empty()) {
           ++status.empty_response_count;
           return openrtb::BidResponse();
        }

        if (responses.size() == config.data().num_bidders) {
            ++status.all_response_count;
        } else {
            ++status.timeout_response_count;
        }
        std::sort(responses.begin(), responses.end(), [](const openrtb::BidResponse &first, const openrtb::BidResponse &second) -> bool {
            if(!first.seatbid.size() || !first.seatbid[0].bid.size()) {
                return false;
            }
            if(!second.seatbid.size() || !second.seatbid[0].bid.size()) {
                return true;
            }
            return first.seatbid[0].bid[0].price > second.seatbid[0].bid[0].price;
        }); // sort by first imp bid?
        return std::move(responses[0]); 
    });
    
    
    connection_endpoint ep {std::make_tuple(config.get("multi_exchange.host"), config.get("multi_exchange.port"), config.get("multi_exchange.root"))};
    restful_dispatcher_t dispatcher(ep.root) ;
    dispatcher.crud_match(boost::regex("/bid/"))
        .post([&openrtb_handler_distributor](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            openrtb_handler_distributor.handle_post(r,match);
        });
    dispatcher.crud_match(boost::regex("/status.html"))
        .get([&status](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            std::stringstream ss;
            ss << status;
            r << ss.str() ;
            r.stock_reply(http::server::reply::ok);
        });

    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.set_concurrency(config.data().concurrency).run() ;
    return 0;
}

