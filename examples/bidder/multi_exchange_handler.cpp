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
#include "user_info.hpp"
 
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include "redisclient/redisasyncclient.h"
#include <redisclient/redissyncclient.h>

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
using multiexchange_config = vanilla::config::config<multi_exchange_handler_config_data>;

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

namespace vanilla {
    class redis_client_wrapper {
    public:
        using connection_ok_handler_type = std::function<void(boost::asio::io_service&)>;
        using connection_fail_handler_type = std::function<void(const std::string&, boost::asio::io_service&)>;
        using get_handler_type = std::function<void(boost::asio::io_service&, const std::string&)>;
    
        redis_client_wrapper(boost::asio::io_service &io):
            io{io}, client{io}
        {}
        void connect(const std::string &host, uint16_t port, const connection_ok_handler_type &connection_ok_handler, const connection_fail_handler_type &connection_fail_handler) {
            client.connect(boost::asio::ip::address::from_string(host), port, [this, &connection_ok_handler, &connection_fail_handler](bool ok, const std::string &err) {
                if(!ok) {
                    connection_fail_handler(err, this->io);
                }
                else {
                    connection_ok_handler(this->io);
                }
            });
        }
        void get(const std::string &key, std::string &data, const get_handler_type &get_handler) {
            client.command("GET", {key}, [this, &data, &get_handler](const redisclient::RedisValue &redis_value) {
                data = redis_value.toString();
                get_handler(this->io, data);    
            });
        }
        bool connected() const {
            return client.isConnected();
        }
    private:
        boost::asio::io_service &io;
        redisclient::RedisAsyncClient client;
    };
    template <typename Wrapper>
    class key_value_client {
    public:
        using self_type = key_value_client;
        using response_handler_type = std::function<void(void)>;
    
        key_value_client():
            client{io}
        {}
        
        self_type &response(const response_handler_type &handler) {
            response_handler = handler;
            return *this;
        }
        
        void request(const std::string &key, std::string &data) {
            LOG(debug) << "Redis make GET request on " << key;
            
            client.get(key, data, [](boost::asio::io_service& io, const std::string& data){
                LOG(debug) << "got value \"" << data << "\"";
                io.stop();    
            });
            
            io.reset();
            io.run();
            
            response_handler();
        }
        
        void connect(const std::string &host, uint16_t port) {
            LOG(debug) << "Connection " << host << ":" << port;
            client.connect(host, port, 
                [](boost::asio::io_service& io) {
                    LOG(debug) << "Connection done";
                    io.stop(); 
                },
                [](const std::string& err, boost::asio::io_service& io) {
                    LOG(error) << "Failed to connect redis (" << err << ")";
                    io.stop(); 
                }
            );
            io.run();
        }
        bool connected() const {
            return client.connected();
        }
        
        private:
            boost::asio::io_service io;
            Wrapper client;
            response_handler_type response_handler;
    };

    class multibidder_collector {
    public:
        multibidder_collector(const multiexchange_config& config, multi_exchange_status &status) :
            config{config}, status{status}
        {}
    
        openrtb::BidResponse getResponse() {
            updateStatus();
            if(responses.empty()) {
                return openrtb::BidResponse(); 
            }
            std::sort(responses.begin(), responses.end(), [](const openrtb::BidResponse &first, const openrtb::BidResponse & second) -> bool {
                if (!first.seatbid.size() || !first.seatbid[0].bid.size()) {
                    return false;
                }
                if (!second.seatbid.size() || !second.seatbid[0].bid.size()) {
                    return true;
                }
                return first.seatbid[0].bid[0].price > second.seatbid[0].bid[0].price;
            }); // sort by first imp bid?
            return responses[0]; 
        }
        bool isDone() const {
            return responses.size() == config.data().num_bidders;
        }
        
        void add(openrtb::BidResponse && bid) {
            ++status.bidder_response_count;
            responses.emplace_back(bid);
        }
    private:
        void updateStatus() {
            if (responses.empty()) {
                ++status.empty_response_count;
            }
            else if(responses.size() == config.data().num_bidders) {
                 ++status.all_response_count;
        
            } else {
                ++status.timeout_response_count;
            }
        }
        const multiexchange_config &config;
        multi_exchange_status &status;
        std::vector<openrtb::BidResponse> responses;
    };
    
    class multibidder_communicator {
    public:
        multibidder_communicator(const multiexchange_config& config) :
            config{config}
        {
            communicator.outbound(config.data().bidders_port);
        }

        void process(const vanilla::VanillaRequest &vanilla_request, multibidder_collector &collector) {
            communicator
                .distribute(vanilla_request)
                .collect<openrtb::BidResponse>(std::chrono::milliseconds(config.data().bidders_response_timeout), [&collector](openrtb::BidResponse bid, auto done) { //move ctored by collect()    
                    collector.add(std::move(bid));
                    if(collector.isDone()) {
                        done();
                    }
                });
        }
    private:
        vanilla::messaging::communicator<vanilla::messaging::broadcast> communicator;
        const multiexchange_config &config;
    };
    
    
}
/*
auto collect_bidders_responses_f = [&]() -> void {
            communicator
            .distribute(vanilla_request)
            .collect<openrtb::BidResponse>(std::chrono::milliseconds(config.data().bidders_response_timeout), [&responses, &config, &status](openrtb::BidResponse bid, auto done) { //move ctored by collect()    
                responses.emplace_back(std::move(bid));
                ++status.bidder_response_count;
                if (responses.size() == config.data().num_bidders) {
                    done();
                }
            });
        };*/
int main(int argc, char* argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using namespace vanilla::exchange;
    namespace po = boost::program_options;   
    multiexchange_config config([&](multi_exchange_handler_config_data &d, po::options_description &desc){
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
    //.auction([&config, &status/*, &redis, &ioService*/, &kv_pool](const openrtb::BidRequest &request) {
    .auction_async([&config, &status](const openrtb::BidRequest &request) {
        using namespace vanilla::messaging;
        ++status.request_count;
        
        
        vanilla::VanillaRequest vanilla_request;
        vanilla_request.bid_request = request; // optimize
        
        if(request.user) {
            vanilla_request.user_info.user_id = request.user.get().buyeruid;
        }
        thread_local vanilla::multibidder_communicator communicator(config);
        vanilla::multibidder_collector collector(config, status);
        
        using kv_type = vanilla::key_value_client<vanilla::redis_client_wrapper>;
        thread_local kv_type kv_client;
        
        //auto kv_client = kv_pool.get();
        if(!kv_client.connected()) {
            LOG(debug) << "kv not connected";
            kv_client.connect("127.0.0.1", 6379);
        }
        bool is_matched_user = vanilla_request.user_info.user_id.length();
        
        if(!is_matched_user || !kv_client.connected()) { // it's not available at all
            LOG(debug) << "Redis is not connected yeat";
            communicator.process(vanilla_request, collector);
        }
        else {
            kv_client
                .response([&vanilla_request, &collector](){
                    communicator.process(vanilla_request, collector);
                })
                .request(vanilla_request.user_info.user_id, vanilla_request.user_info.user_data);
            
            //kv_pool.push(kv_client); // return it back
        }
        //return openrtb::BidResponse();
        return collector.getResponse();
        
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

