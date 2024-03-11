#include <vector>
#include <random>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "rtb/core/core.hpp"
#include "rtb/exchange/exchange_handler.hpp"
#include "rtb/exchange/exchange_server.hpp"
#include "rtb/DSL/generic_dsl.hpp"
#include "rtb/DSL/extractors.hpp"
#include "rtb/DSL/any_mapper.hpp"
#include "rtb/DSL/rapid_mapper.hpp"
#include "rtb/config/config.hpp"
#include "rtb/core/tagged_tuple.hpp"
#include "rtb/datacache/entity_cache.hpp"
#include "rtb/datacache/memory_types.hpp"
#include "rtb/datacache/generic_bidder_cache_loader.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"


#include "examples/matchers/ico_campaign.hpp"
#include "examples/matchers/domain.hpp"
#include "examples/matchers/ad.hpp"
#include "examples/campaign/campaign_cache.hpp"

#include "rtb/core/bidder.hpp"
#include "rtb/core/ad_selector.hpp"


extern void init_framework_logging(const std::string &) ;

using namespace vanilla;

int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    using DSLT = DSL::GenericDSL<std::string, DSL::rapid_mapper> ;
    using BidRequest = DSLT::deserialized_type;
    // using BidResponse = DSLT::serialized_type;
    using BidderConfig = vanilla::config::config<ico_bidder_config_data>;
    using CacheLoader  =  vanilla::GenericBidderCacheLoader<DomainEntity<>, ICOCampaignEntity<>, AdDataEntity<BidderConfig>, CampaignCache<BidderConfig>>;
    using Selector = vanilla::ad_selector<vanilla::BudgetManager, Ad>;

    BidderConfig config([](ico_bidder_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("ico-bidder.log", boost::program_options::value<std::string>(&d.log_file_name), "bidder_test log file name log")
            ("ico_bidder.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("data/ico_ads"), "ads_source file name")
            ("ico_bidder.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
            ("ico-bidder.domain_source", boost::program_options::value<std::string>(&d.domain_source)->default_value("data/ico_domains"), "domain_source file name")
            ("ico-bidder.domain_ipc_name", boost::program_options::value<std::string>(&d.domain_ipc_name)->default_value("vanilla-domain-ipc"), "domain ipc name")
            ("ico-bidder.port", boost::program_options::value<short>(&d.port)->required(), "ico_bidder port")
            ("ico-bidder.host", boost::program_options::value<std::string>(&d.host)->default_value("0.0.0.0"), "ico_bidder host")
            ("ico-bidder.root", boost::program_options::value<std::string>(&d.root)->default_value("."), "ico_bidder root")
            ("ico-bidder.timeout", boost::program_options::value<int>(&d.timeout), "ico_bidder timeout")
            ("ico-bidder.concurrency", boost::program_options::value<unsigned int>(&d.concurrency)->default_value(0), "ico_bidder concurrency, if 0 is set std::thread::hardware_concurrency()")
            ("ico-bidder.ico_campaign_ipc_name", boost::program_options::value<std::string>(&d.ico_campaign_ipc_name)->default_value("vanilla-ico-campaign-ipc"), "ico campaign ipc name")
            ("ico-bidder.ico_campaign_source", boost::program_options::value<std::string>(&d.ico_campaign_source)->default_value("data/ico_campaign"), "ico_campaign_source file name")
            ("campaign-manager.ipc_name", boost::program_options::value<std::string>(&d.ipc_name),"campaign_budget IPC name")
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

    CacheLoader cacheLoader(config);

    try {
        cacheLoader.load(); // Not needed if data cache loader is in work

    }
    catch(std::exception const& e) {
        LOG(error) << e.what();
        return 0;
    }
    
    using bid_handler_type = exchange_handler<DSLT>;

    bid_handler_type bid_handler(std::chrono::milliseconds(config.data().timeout));

    // NB: in production code you might want to use boost::small_vec with N=8..32
    using campaign_vec_t = std::vector<ICOCampaign>;
    using ad_vec_t = std::vector<Ad>;

    //Return from each lambda becomes input for next lambda in the tuple of functions
    auto retrieve_domain_f = [&cacheLoader](const std::string& domain_str, auto&& ...) {
        Domain domain;
 
        [[maybe_unused]] bool retrieved = cacheLoader.retrieve(domain, domain_str);
        assert(retrieved == (domain.dom_id != Domain::invalid_domain_id));

        return domain.dom_id;
    };

    auto retrieve_ico_campaign_f = [&cacheLoader](Domain::dom_id_t dom_id, auto&& ...)  {
        campaign_vec_t ico_campains;

        if (dom_id != Domain::invalid_dom_id) {
            [[maybe_unused]] bool retrieved = cacheLoader.retrieve(ico_campains, dom_id);
            assert(retrieved || ico_compains.empty());
        }

        return ico_campains;
    };

    vanilla::core::Banker<BudgetManager> banker;
    auto retrieve_campaign_ads_f = [&](campaign_vec_t campaigns, [[maybe_unused]] auto && req, auto && imp)  {
        ad_vec_t ads;
        for (auto &campaign : campaigns) {
            if (cacheLoader.retrieve(ads, campaign.campaign_id, imp.banner.get().w, imp.banner.get().h)) {
                auto budget_bid = banker.authorize(cacheLoader.get<CampaignCache<BidderConfig>>(), campaign.campaign_id);
                for_each(begin(ads), end(ads), [budget_bid](Ad& ad) {
                    ad.auth_bid_micros = std::min(budget_bid, ad.max_bid_micros);
                });
            }
        }
        return ads;
    };

    bid_handler    
        .logger([]([[maybe_unused]] const std::string &data) {
            //LOG(debug) << "bid request=" << data ;
        })
        .error_logger([](const std::string &data) {
            LOG(debug) << "bid request error " << data ;
        })
        .auction_async([&](const BidRequest &request) {
            thread_local vanilla::Bidder<DSLT, Selector> bidder{Selector{}};
            return bidder.bid(request,
                              //chained matchers
                              request.site.get().ref,
                              retrieve_domain_f,
                              retrieve_ico_campaign_f,
                              retrieve_campaign_ads_f
            );
        });
    
    connection_endpoint ep {std::make_tuple(config.data().host, boost::lexical_cast<std::string>(config.data().port), config.data().root)};

    //initialize and setup CRUD dispatcher
    restful_dispatcher_t dispatcher(ep.root);
    dispatcher.crud_match(boost::regex("/ico_bid/(\\d+)"))
            .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                bid_handler.handle_post(r, match);
            });

    LOG(debug) << "concurrency " << config.data().concurrency;
    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.set_concurrency(config.data().concurrency).run() ;
}


