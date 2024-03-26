/*
 * File:   bidder_caches.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:15
 * Updated on 03/25/2024 utilizing MultiCacheLoader
 */

#ifndef BIDDER_CACHES_HPP
#define BIDDER_CACHES_HPP

#include "examples/campaign/campaign_cache.hpp"
#include "examples/matchers/ad.hpp"
#include "examples/matchers/geo.hpp"
#include "examples/matchers/geo_campaign.hpp"
#include "rtb/common/perf_timer.hpp"
#include "rtb/core/openrtb.hpp"
#include "rtb/datacache/multi_cache_loader.hpp"

namespace vanilla {

// clang-format off
template <typename Config>
using BidderCachesLoader = vanilla::MultiCacheLoader<
  AdDataEntity<Config>,
  GeoDataEntity<Config>,
  GeoCampaignEntity<Config>,
  vanilla::CampaignCache<Config>>;
// clang-format on

template <typename Config = BidderConfig> struct BidderCaches : private BidderCachesLoader<Config> {
    //clang-format off
    explicit BidderCaches(Config const& config)
        : BidderCachesLoader<Config>{config}, config{config},
          ad_data_entity{BidderCachesLoader<Config>::template get<AdDataEntity<Config>>()},
          geo_data_entity{BidderCachesLoader<Config>::template get<GeoDataEntity<Config>>()},
          geo_campaign_entity{BidderCachesLoader<Config>::template get<GeoCampaignEntity<Config>>()},
          budget_cache{BidderCachesLoader<Config>::template get<vanilla::CampaignCache<Config>>()}
    {}
    // clang-format on

    using BidderCachesLoader<Config>::load;
    using BidderCachesLoader<Config>::retrieve;
    using BidderCachesLoader<Config>::get;

    Config const& config;
    AdDataEntity<Config>& ad_data_entity;
    GeoDataEntity<Config>& geo_data_entity;
    GeoCampaignEntity<Config>& geo_campaign_entity;
    vanilla::CampaignCache<Config>& budget_cache;
};
} // namespace vanilla

#endif /* BIDDER_CACHES_HPP */
