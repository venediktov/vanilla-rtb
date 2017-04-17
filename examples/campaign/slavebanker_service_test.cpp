
/* 
 * File:   slavebanker_service_test.cpp
 * Author: vladimir venediktov
 *
 * Created on March 29, 2017, 9:47 PM
 */

#include <string>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include "rtb/config/config.hpp"
#include "rtb/messaging/communicator.hpp"
#include "campaign_cache.hpp"
#include "serialization.hpp"

#include "rtb/core/core.hpp"

extern void init_framework_logging(const std::string &) ;

using namespace vanilla;

template<typename Cache>
void run(short port, Cache &cache) {
    using namespace vanilla::messaging;
    communicator<broadcast>().inbound(port).consume<CampaignBudget>([&cache](auto endpoint, auto budget) {
        cache.update(budget,budget.campaign_id);
        LOG(debug) << "updated budget :" << budget;
        return ;
    }).dispatch();
}

int main(int argc, char** argv) {
    using CampaignCacheType  = CampaignCache<SlaveBankerConfig>;
    using CampaignBudgets = typename CampaignCacheType::DataCollection;
    namespace po = boost::program_options;   
    vanilla::config::config<slavebanker_service_config_data> config([](slavebanker_service_config_data &d, po::options_description &desc){
        desc.add_options()
            ("slave-banker-service.log", po::value<std::string>(&d.log_file_name), "slavebanker_service_test log file name")
            ("multi_bidder.budget_port", po::value<int>(&d.budget_port)->required(), "udp port for broadcast to bidders budget change")
            ("slave-banker-service.ipc_name", po::value<std::string>(&d.ipc_name)->required(), "name of campaign manager ipc cache")
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
    run(config.data().budget_port, cache);
    
    return 0;
}

