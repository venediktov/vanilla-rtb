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
#include "user_info.hpp"
 
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include "redisclient/redisasyncclient.h"
#include <redisclient/redissyncclient.h>

#include "multiexchange_config.hpp"
#include "multiexchange_status.hpp"

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

namespace vanilla {
    class redis_client_adapter {
    public:
        using connection_ok_handler_type = std::function<void(boost::asio::io_service&)>;
        using connection_fail_handler_type = std::function<void(const std::string&, boost::asio::io_service&)>;
        using get_handler_type = std::function<void(boost::asio::io_service&, const std::string&)>;
    
        redis_client_adapter(boost::asio::io_service &io):
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
    class asio_key_value_client {
    public:
        using self_type = asio_key_value_client;
        using response_handler_type = std::function<void(void)>;
    
        asio_key_value_client():
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
        using response_handler_type = std::function<void(const multibidder_collector*)>;
        using add_handler_type = std::function<void(const multibidder_collector*)>;
        using self_type = multibidder_collector;
        using responses_type = std::vector<openrtb::BidResponse>;
                
        multibidder_collector(int num_bidders) :
            num_bidders{num_bidders}
        {}
        
        self_type &on_response(const response_handler_type &response_handler_) {
            response_handler = response_handler_;
            return *this;
        }    
        self_type &on_add(const add_handler_type & add_handler_) {
            add_handler = add_handler_;
            return *this;
        }    
        
        openrtb::BidResponse response() {
            if(response_handler) {
                response_handler(this);
            }
            
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

        
        bool done() const {
            return responses.size() == num_bidders;
        }
        self_type & clear() {
            responses.clear();
            return *this;
        }
        
        void add(openrtb::BidResponse && bid) {
            if(add_handler) {
                add_handler(this);
            }
            responses.emplace_back(bid);
        }
        
        const responses_type &get_reponses() const {
            return responses;
        }
        
        int get_num_bidders() const {
            return num_bidders;
        }
    private:
        int num_bidders;
        
        responses_type responses;
        response_handler_type response_handler;
        add_handler_type add_handler;
    };
    
    class multibidder_communicator {
    public:
        multibidder_communicator(const vanilla::multiexchange::multiexchange_config& config) :
            config{config}
        {
            communicator.outbound(config.data().bidders_port);
        }

        void process(const vanilla::VanillaRequest &vanilla_request, multibidder_collector &collector) {
            communicator
                .distribute(vanilla_request)
                .collect<openrtb::BidResponse>(std::chrono::milliseconds(config.data().bidders_response_timeout), [&collector](openrtb::BidResponse bid, auto done) { //move ctored by collect()    
                    collector.add(std::move(bid));
                    if(collector.done()) {
                        done();
                    }
                });
        }
    private:
        vanilla::messaging::communicator<vanilla::messaging::broadcast> communicator;
        const vanilla::multiexchange::multiexchange_config &config;
    };
    
    
}

int main(int argc, char* argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using namespace vanilla::exchange;
    using namespace vanilla::multiexchange;
    namespace po = boost::program_options;   
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
    vanilla::exchange::exchange_handler<DSL::GenericDSL> openrtb_handler_distributor(std::chrono::milliseconds(config.data().handler_timeout));
    openrtb_handler_distributor
    .logger([](const std::string &data) {
        //LOG(debug) << "request_data for distribution=" << data ;
    })
    .error_logger([](const std::string &data) {
        LOG(debug) << "request for distribution error " << data ;
    })
    
    .auction_async([&config, &status](const openrtb::BidRequest &request) {
        using namespace vanilla::messaging;
        ++status.request_count;
        
        
        vanilla::VanillaRequest vanilla_request;
        vanilla_request.bid_request = request; // optimize
        
        if(request.user) {
            vanilla_request.user_info.user_id = request.user.get().buyeruid;
        }
        thread_local vanilla::multibidder_communicator communicator(config);
        thread_local vanilla::multibidder_collector collector(config.data().num_bidders);
        collector
            .clear()
            .on_response([&status](const vanilla::multibidder_collector* collector) {
                auto &r = collector->get_reponses();
                if (r.empty()) {
                    ++status.empty_response_count;
                } else if (r.size() == collector->get_num_bidders()) {
                    ++status.all_response_count;
                } else {
                    ++status.timeout_response_count;
                }
            })
            .on_add([&status](const vanilla::multibidder_collector* collector) {
                 ++status.bidder_response_count;
            });
        
        using kv_type = vanilla::asio_key_value_client<vanilla::redis_client_adapter>;
        thread_local kv_type kv_client;
        
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
                .response([&vanilla_request/*, &collector*/](){
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
    server.set_concurrency(config.data().concurrency).run() ;
    return 0;
}

