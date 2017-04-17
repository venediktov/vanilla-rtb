/* 
 * File:   campaign_manager_test.cpp
 * Author: vladimir venediktov
 *
 * Created on March 12, 2017, 10:22 PM
 * 
 *  General idea is to map CRUD commands to functions
 * 
 *  campaign_budget { campaign_id , campaign_budget }
 *  PUT  /v1/campaign/budget/id/123
 *  PUT  /v1/campaign/budget/id/456
 *  GET  /v1/campaign/budget/ -> {[{id:123}, {id:456}]}
 *  GET  /v1/campaign/budget/id/123 -> {{id:123}}
 *  POST /v1/campaign/budget/id/123
 *  DEL  /v1/campaign/budget/id/123
 * 
 *  cmapign_site { campaign_id , campaign_site} 
 *  GET /v1/campaign/site/id/123 
 *  GET /v1/campaign/site/name/some-name
 *
 *  campaign_geo  { campaign_id , campaign_city , campaign_country }
 *  GET /v1/campaign/geo/id/123
 *  GET /v1/campaign/geo/name/some-name
 *  GET /v1/campaign/os/id/123
 *  GET /v1/campaign/os/name/some-name
 * 
 *  GET /v1/campaign/123  ->  campaign :
 *                            {
 *                             "id" : "123"
 *                              campaign_budget :
 *                              {
 *                                 "budget":1000000,
 *                                 "cpc":300000,
 *                                 "cpm":20000,
 *                                 "id":123,
 *                                 "spent":1000
 *                              },
 *                              site :
 *                              {
 *                                "id":"123", 
 *                                "name" : null
 *                              },
 *                              os : {
 *                                "id":123, 
 *                                "name":null
 *                              }
 *                           }
 *
 *
 *  GET /v1/campaign/budget/123 -> will return budget object associated with this compaign
 * 
 *  How it can be compactly achieved utilizing our CRUD regexes 
 *  We know it's possible to retrieve data by unique key or composite key from the data store
 *  to achieve that \\d+ requires 1 or more digits , but \\d* means 0 or more , so the regex group can be empty 
 *  
 *  then we need to figure out in those mapping function keys and type of keys for all 
 *  other type of campaign structures not only budget
 *  for now just budget cache by campaign_id will be coded but ideally it should be one single map
 *  of CRUD handlers to cache functions
 * 
 *  std::map<std::string, std::function<void(CampaignBudgets&,uint32_t)>> read_commands = {
 *       {"id"   , [&cache](auto &data, auto id){cache.retrieve(data,id);}},
 *       {"site" , [&cache](auto &data, auto site){cache.retrieve(data,site);}},
 *       {"geo"  , [&cache](auto &data, auto geo){cache.retrieve(data,geo);}},
 *       {"os"   , [&cache](auto &data, auto os){cache.retrieve(data,os);}}
 *   };
 */

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include "CRUD/service/server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "DSL/campaign_dsl.hpp"
#include "config.hpp"
#include "campaign_cache.hpp"
#include "serialization.hpp"


#include "rtb/core/core.hpp"

extern void init_framework_logging(const std::string &) ;


int main(int argc, char *argv[]) {
    using namespace vanilla;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using CampaignCacheType  = CampaignCache<CampaignManagerConfig>;
    using CampaignBudgets = typename CampaignCacheType::DataCollection;
    
//    std::string Create;
//    std::string Read;
//    std::string Update;
//    std::string Delete;
 
    CampaignManagerConfig config([](campaign_manager_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("campaign-manager.log", boost::program_options::value<std::string>(&d.log_file_name), "campaign_manager_test log file name log")
            ("campaign-manager.host", "campaign_manager_test Host")
            ("campaign-manager.port", "campaign_manager_test Port")
            ("campaign-manager.root", "campaign_manager_test Root")
            ("campaign-manager.ipc_name", boost::program_options::value<std::string>(&d.ipc_name),"campaign_manager_test IPC name")
            ("campaign-manager.budget_source", boost::program_options::value<std::string>(&d.campaign_budget_source)->default_value("data/campaign_budget"),"campaign_budget source file name")
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
    try {
        cache.load();
    }
    catch(std::exception const& e) {
        LOG(error) << e.what(); // It still can work without preloaded campaign budgets
    }
 
    std::map<std::string, std::function<void(const CampaignBudget&,uint32_t)>> create_commands = {
        {"budget/id/" , [&cache](auto cb, auto id){cache.insert(cb,id);}}
    };
    std::map<std::string, std::function<void(CampaignBudgets&,uint32_t)>> read_commands = {
        {"budget/id/" , [&cache](auto &data, auto id){cache.retrieve(data,id);}},
        {"budget" ,   [&cache](auto &data, auto id){cache.retrieve(data);}}
    };
    std::map<std::string, std::function<void(const CampaignBudget&,uint32_t)>> update_commands = {
        {"budget/id/" , [&cache](auto cb, auto id){cache.update(cb,id);}}
    };
    std::map<std::string, std::function<void(uint32_t)>> delete_commands = {
        {"budget/id/" , [&cache](auto id){cache.remove(id);}}
    };
    //initialize and setup CRUD dispatcher
    restful_dispatcher_t dispatcher(config.get("campaign-manager.root")) ;
    dispatcher.crud_match(boost::regex("/campaign/([A-Za-z/]+)(\\d+)"))
              .put([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
              LOG(info) << "Create received cache update event url=" << match[0];
                try {
                    auto data = DSL::CampaignDSL<CampaignBudget>().extract_request(match.data);
                    uint32_t campaign_id = boost::lexical_cast<uint32_t>(match[2]);
                    create_commands[match[1]](data, campaign_id);
                } catch (std::exception const& e) {
                    LOG(error) << e.what();
                }
              })
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                LOG(info) << "Update received event url=" << match[0];
                try {
                    auto data = DSL::CampaignDSL<CampaignBudget>().extract_request(match.data);
                    uint32_t campaign_id = boost::lexical_cast<uint32_t>(match[2]);
                    update_commands[match[1]](data,campaign_id);
                } catch (std::exception const& e) {
                    LOG(error) << e.what();
                }
              })
              .del([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                LOG(info) << "Delete received event url=" << match[0];
                try {
                    uint32_t campaign_id = boost::lexical_cast<uint32_t>(match[2]);
                    delete_commands[match[1]](campaign_id);
                } catch (std::exception const& e) {
                    LOG(error) << e.what();
                }
    });
    dispatcher.crud_match(boost::regex("/campaign/([A-Za-z/]+)(\\d*)"))
              .get([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
              LOG(info) << "Read received event url=" << match[0];
              try {
                  CampaignBudgets data;
                  boost::optional<uint32_t> campaign_id;
                  std::string key   = match[1];
                  std::string value = match[2];
                  if ( value.length() ) {
                      campaign_id = boost::lexical_cast<uint32_t>(value);
                  }
                  read_commands[key](data, campaign_id? *campaign_id : 0 );
                  r<< CampaignBudget::desc();
                  for(auto &d : data) {
                    r << boost::lexical_cast<std::string>(*d) << "\n";
                  }
              } catch (std::exception const& e) {
                  LOG(error) << e.what();
              }
    });


    auto host = config.get("campaign-manager.host");
    auto port = config.get("campaign-manager.port");
    http::server::server<restful_dispatcher_t> server(host,port,dispatcher);
    server.run();
}

