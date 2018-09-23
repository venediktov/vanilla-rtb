#include <benchmark/benchmark.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <rtb/config/config.hpp>
#include <rtb/datacache/memory_types.hpp>
#include <rtb/datacache/entity_cache.hpp>
#include "../examples/datacache/ad_entity.hpp"
#include "../examples/datacache/geo_entity.hpp"
#include "../examples/datacache/city_country_entity.hpp"
#include "../examples/datacache/campaign_entity.hpp"
#include <rtb/common/perf_timer.hpp>
#include "../examples/matchers/ad.hpp"
#include "../examples/matchers/geo.hpp"
#include "../examples/campaign/campaign_cache.hpp"
#include "../examples/matchers/geo_campaign.hpp"
#include "../examples/loader/config.hpp"
#include "../examples/bidder/bidder_caches.hpp"
#include "../examples/bidder/serialization.hpp"

#include <memory>

namespace {

struct CacheBenchmarkFixture: benchmark::Fixture
{
    CacheLoadConfig config_;
    std::unique_ptr<GeoDataEntity<CacheLoadConfig>> geo_cache_;
    std::unique_ptr<AdDataEntity<CacheLoadConfig>> ad_cache_;
    std::unique_ptr<vanilla::CampaignCache<CacheLoadConfig>> campaign_cache_;
    std::unique_ptr<GeoCampaignEntity<CacheLoadConfig>> geo_campaign_cache_;

    vanilla::CampaignBudget campaignBudget_;
    GeoCampaignEntity<CacheLoadConfig>::GeoCampaignCollection geoCampaigns_;

    CacheBenchmarkFixture():
        config_([](cache_loader_config_data &d, boost::program_options::options_description &desc){
            desc.add_options()
                ("cache-loader.log", boost::program_options::value<std::string>(&d.log_file_name), "cache_loader_test log file name log")
                ("cache-loader.host", "cache_loader_test Host")
                ("cache-loader.port", "cache_loader_test Port")
                ("cache-loader.root", "cache_loader_test Root")
                ("datacache.ads_source", boost::program_options::value<std::string>(&d.ads_source)->default_value("data/ads"), "ads_source file name")
                ("datacache.ads_ipc_name", boost::program_options::value<std::string>(&d.ads_ipc_name)->default_value("vanilla-ads-ipc"), "ads ipc name")
                ("datacache.geo_source", boost::program_options::value<std::string>(&d.geo_source)->default_value("data/geo"), "geo_source file name")
                ("datacache.geo_ipc_name", boost::program_options::value<std::string>(&d.geo_ipc_name)->default_value("vanilla-geo-ipc"), "geo ipc name")        
                ("bidder.geo_campaign_ipc_name", boost::program_options::value<std::string>(&d.geo_campaign_ipc_name)->default_value("vanilla-geo-campaign-ipc"), "geo campaign ipc name")
                ("bidder.geo_campaign_source", boost::program_options::value<std::string>(&d.geo_campaign_source)->default_value("data/geo_campaign"), "geo_campaign_source file name")
                ("campaign-manager.ipc_name", boost::program_options::value<std::string>(&d.ipc_name)->default_value("vanilla-campaign-data-ipc"), "campaign data ipc name")
                ("campaign-manager.campaign_budget_source", boost::program_options::value<std::string>(&d.campaign_budget_source)->default_value("data/campaign_budget"), "campaign_budget_source file name")
            ;
        })
    {
        char argv0[] = "";
        char argv1[] = "--config";
        char argv2[] = "config.cfg";
        char* argv[] {argv0, argv1, argv2, nullptr};
        config_.parse(3, argv);
        geo_cache_ = std::make_unique<GeoDataEntity<CacheLoadConfig>>(config_);
        ad_cache_ = std::make_unique<AdDataEntity<CacheLoadConfig>>(config_);
        campaign_cache_ = std::make_unique<vanilla::CampaignCache<CacheLoadConfig>>(config_);
        campaignBudget_ = campaign_cache_->retrieve(36);
        LOG(debug) << "campaign: " << campaignBudget_;
        geo_campaign_cache_ = std::make_unique<GeoCampaignEntity<CacheLoadConfig>>(config_);
        geo_campaign_cache_->retrieve(geoCampaigns_, 564);
        LOG(debug) << "geo_campaign: " << geoCampaigns_;
        boost::log::core::get()->set_logging_enabled(false);
    }
}; // CacheBenchmarkFixture



BENCHMARK_DEFINE_F(CacheBenchmarkFixture, geo_load_benchmark)(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        //benchmark::DoNotOptimize()
        std::make_unique<GeoDataEntity<CacheLoadConfig>>(config_)->load();
    }

    //state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * input.size());
}

BENCHMARK_REGISTER_F(CacheBenchmarkFixture, geo_load_benchmark);


BENCHMARK_DEFINE_F(CacheBenchmarkFixture, ad_load_benchmark)(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        std::make_unique<AdDataEntity<CacheLoadConfig>>(config_)->load();
    }

    //state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * input.size());
}

BENCHMARK_REGISTER_F(CacheBenchmarkFixture, ad_load_benchmark);


BENCHMARK_DEFINE_F(CacheBenchmarkFixture, geo_retrieve_benchmark)(benchmark::State& state)
{
    std::string const city {"novosibirsk"};
    std::string const country {"russia"};
    while (state.KeepRunning())
    {
        Geo geo;
        benchmark::DoNotOptimize(geo_cache_->retrieve(geo, city, country));
    }
}

BENCHMARK_REGISTER_F(CacheBenchmarkFixture, geo_retrieve_benchmark);


struct dummy_key {
    uint32_t const key;

    template <typename T>
    uint32_t get() const { return key; }
};


BENCHMARK_DEFINE_F(CacheBenchmarkFixture, campaign_load_benchmark)(benchmark::State& state)
{
    while (state.KeepRunning())
    {
//        benchmark::DoNotOptimize()
        std::make_unique<vanilla::CampaignCache<CacheLoadConfig>>(config_)->load();
    }

//    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * input.size());
}

BENCHMARK_REGISTER_F(CacheBenchmarkFixture, campaign_load_benchmark);

BENCHMARK_DEFINE_F(CacheBenchmarkFixture, campaign_retrieve_benchmark)(benchmark::State& state)
{
    uint32_t const campaign_id = 36;
    vanilla::CampaignBudget campaignBudget;
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(campaign_cache_->retrieve(campaign_id));
    }
}

BENCHMARK_REGISTER_F(CacheBenchmarkFixture, campaign_retrieve_benchmark);

BENCHMARK_DEFINE_F(CacheBenchmarkFixture, geo_campaign_load_benchmark)(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        std::make_unique<GeoCampaignEntity<CacheLoadConfig>>(config_)->load();
    }
}

BENCHMARK_REGISTER_F(CacheBenchmarkFixture, geo_campaign_load_benchmark);

BENCHMARK_DEFINE_F(CacheBenchmarkFixture, geo_campaign_retrieve_benchmark)(benchmark::State& state)
{
    uint32_t const geo_id = 564;
    GeoCampaignEntity<CacheLoadConfig>::GeoCampaignCollection geoCampaigns;
    while (state.KeepRunning())
    {
        geoCampaigns.clear();
        benchmark::DoNotOptimize(geo_campaign_cache_->retrieve(geoCampaigns, geo_id));
    }
}

BENCHMARK_REGISTER_F(CacheBenchmarkFixture, geo_campaign_retrieve_benchmark);

} // local namespace

