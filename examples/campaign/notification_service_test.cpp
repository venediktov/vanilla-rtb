/* 
 * File:   notification_service_ex.cpp
 * Author: vladimir venediktov
 *
 * Created on March 26, 2017, 11:31 AM
 */

#include <string>
#include <iostream>
#include <tuple>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "CRUD/service/server.hpp"
#include "rtb/config/config.hpp"
#include "rtb/messaging/communicator.hpp"
#include "core/tagged_tuple.hpp"
#include "campaign_cache.hpp"
#include "serialization.hpp"


#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;



struct notification_service_status {
    boost::posix_time::ptime start{boost::posix_time::microsec_clock::local_time()};
    boost::atomic_uint64_t win_notice_count{};
    boost::atomic_uint64_t day_budget_limit{};
    boost::atomic_uint64_t total_spent_amout{};

    friend std::ostream& operator<<(std::ostream &os, const notification_service_status &st) {
        boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - st.start;
        os << "start: " << boost::posix_time::to_simple_string(st.start) << "<br/>" <<
              "elapsed: " << boost::posix_time::to_simple_string(td) << "<br/>" <<
              "<table border=0>" <<
              "<tr><td>win notice count</td><td>" << st.win_notice_count << "</td></tr>" <<
              "<tr><td>daily budget limit</td><td>" << st.day_budget_limit << "</td></tr>" <<
              "<tr><td>total spent amount</td><td>" << st.total_spent_amout << "</td></tr>" << 
              "</table> ";
        return os;
    }
};

struct connection_endpoint {
    using ctor_type = std::tuple<std::string, std::string, std::string>;
    std::string host;
    std::string port;
    std::string root;
    connection_endpoint(const ctor_type &ep) : host{ std::get<0>(ep) }, port{ std::get<1>(ep) }, root{ std::get<2>(ep) }
    {}
   
};

int main(int argc, char* argv[]) {
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using CampaignCacheType  = CampaignCache<WinNotificationConfig>;
    using CampaignBudgets = typename CampaignCacheType::DataCollection;
    namespace po = boost::program_options;   
    vanilla::config::config<notification_service_config_data> config([&](notification_service_config_data &d, po::options_description &desc){
        desc.add_options()
            ("notification-service.log", po::value<std::string>(&d.log_file_name), "exchange_handler_test log file name log")
            ("notification-service.host", "notification_service_test Host")
            ("notification-service.port", "notification_service_test Port")
            ("notification-service.root", "notification_service_test Root")
            ("notification-service.nurl_match", po::value<std::string>(&d.nurl_match)->required(), "matching CRUD path to exchange nurl")
            ("multi_bidder.budget_port", po::value<int>(&d.budget_port)->required(), "udp port for broadcast to bidders budget change")
            ("campaign-manager.ipc_name", po::value<std::string>(&d.ipc_name)->required(), "name of campaign manager ipc cache")
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
    
    CampaignCacheType  cache(config);
     
    // status 
    notification_service_status status;
    
    connection_endpoint ep {std::make_tuple(config.get("notification-service.host"), 
                                            config.get("notification-service.port"), 
                                            config.get("notification-service.root"))
    };
    restful_dispatcher_t dispatcher(ep.root) ;
    //win notice handler
    dispatcher.crud_match(boost::regex(config.data().nurl_match))
        .get([&status,&cache,&config](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            using namespace vanilla::messaging;
            boost::optional<uint32_t> campaign_id;
            boost::optional<uint64_t> price;
            std::string price_str = match[1];
            std::string campaign_id_str = match[2];
            if ( !campaign_id_str.empty() ) {
                campaign_id = boost::lexical_cast<uint32_t>(campaign_id_str);
            }
            if ( !price_str.empty() ) {
                price = boost::lexical_cast<uint64_t>(price_str);
            }
            if ( !campaign_id || !price ) {
                LOG(error) << "failed to parse campaign_id or price from win notification :" << match[0];
                r.stock_reply(http::server::reply::ok);
                return;
            }
            CampaignBudgets budgets;
            if ( !cache.retrieve(budgets, *campaign_id) ) {
                LOG(error) << "failed to get campaign_id=" << *campaign_id << " from cache !";
                r.stock_reply(http::server::reply::ok);
                return;
            }
            LOG(info) << "received win notification campaign_id=" << *campaign_id << " win price=" << *price;
            std::shared_ptr<CampaignBudget> budget_ptr = budgets.at(0);
            budget_ptr->day_budget_limit -= *price;
            budget_ptr->day_budget_spent += *price;
            status.day_budget_limit  = budget_ptr->day_budget_limit;
            status.total_spent_amout = budget_ptr->day_budget_spent;
            ++status.win_notice_count;
            cache.update(*budget_ptr,*campaign_id);
            communicator<broadcast>()
            .outbound(config.data().budget_port)
            .distribute(*budget_ptr);
            r.stock_reply(http::server::reply::ok);
        });
    dispatcher.crud_match(boost::regex("/status.html"))
        .get([&status](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
            r << boost::lexical_cast<std::string>(status) ;
            r.stock_reply(http::server::reply::ok);
        });

    http::server::server<restful_dispatcher_t> server(ep.host, ep.port, dispatcher);
    server.run();
    return 0;
}


