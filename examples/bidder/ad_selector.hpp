/* 
 * File:   bidder_selector.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:15
 */

#ifndef BIDDER_SELECTOR_HPP
#define BIDDER_SELECTOR_HPP

#include "rtb/core/openrtb.hpp"
#include "rtb/core/banker.hpp"
#include "examples/campaign/campaign_cache.hpp"
#include "bidder_caches.hpp"
#include <memory>
#include <algorithm>

namespace vanilla {
template<typename Config = BidderConfig>
class AdSelector {
    public:
        using SpecBidderCaches = BidderCaches<Config>;
        using AdPtr = std::shared_ptr<Ad>;
        using AdSelectionAlg = std::function<AdPtr(const std::vector<Ad>&)>;
        using self_type = AdSelector<Config>;
        
        explicit AdSelector(BidderCaches<Config> &bidder_caches):
            bidder_caches{bidder_caches}
        {
            retrieved_cached_ads.reserve(500);
        }
            
        self_type& alg(const AdSelectionAlg &selection_alg_) {
            selection_alg = selection_alg_;
            return *this;
        }
        
        template<typename T> 
        AdPtr select(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp) {
            AdPtr result;
            
            Geo geo;
            if(!getGeo(req, geo) ) {
                LOG(debug) << "No geo";
                return result;
            }
            
            if(!getGeoCampaigns(geo.geo_id)) {
                LOG(debug) << "No campaigns for geo " << geo.geo_id;
                return result;
            }
            else {
                LOG(debug) << "Selected campaigns " << geo_campaigns.size();
            }
            
            if(!getCampaignAds(geo_campaigns, imp)) {
                LOG(debug) << "No ads for geo " << geo.geo_id;
                return result;
            }
            else {
                LOG(debug) << "selected ads " << retrieved_cached_ads.size();
            }
            
            if(selection_alg) {
                return selection_alg(retrieved_cached_ads);
            }
            return max_bid(retrieved_cached_ads);
        }
        
        bool getGeoCampaigns(uint32_t geo_id) {
            geo_campaigns.clear();
            if (!bidder_caches.retrieve(geo_campaigns, geo_id)) {
                LOG(debug) << "GeoAd retrieve failed " << geo_id;
                return false;
            }
            return true;
        }
        template <typename T>
        bool getGeo(const openrtb::BidRequest<T> &req, Geo &geo) {
            if (!req.user) {
                LOG(debug) << "No user";
                return true;
            }
            if (!req.user.get().geo) {
                LOG(debug) << "No user geo";
                return true;
            }
           
            auto &city_view = req.user.get().geo.get().city ;
            auto &country_view = req.user.get().geo.get().country;
            const std::string city = boost::algorithm::to_lower_copy(std::string(city_view.data(), city_view.size()));
            const std::string country = boost::algorithm::to_lower_copy(std::string(country_view.data(), country_view.size()));

            if (!bidder_caches.retrieve(geo, city, country)) {
                LOG(debug) << "retrieve failed " << city << " " << country;
                return false;
            } 
            return true;
        }

        template <typename T>
        bool getCampaignAds(const std::vector<GeoCampaign> &campaigns, const openrtb::Impression<T> &imp) {
            retrieved_cached_ads.clear();
            for (auto &campaign : campaigns) {
                auto offset = retrieved_cached_ads.size();
                if (!bidder_caches.retrieve(retrieved_cached_ads, campaign.campaign_id, imp.banner.get().w, imp.banner.get().h)) {
                    continue;
                }
                auto budget_bid = authorize(campaign.campaign_id);
                std::transform(std::begin(retrieved_cached_ads) + offset,
                               std::end(retrieved_cached_ads), 
                               std::begin(retrieved_cached_ads) + offset, [budget_bid](Ad & ad){
                                   ad.auth_bid_micros = std::min(budget_bid, ad.max_bid_micros);
                                   return ad;
                               });
            }
            return retrieved_cached_ads.size() > 0;
        }
        
        AdPtr max_bid(const std::vector<Ad>& ads) {
            if(ads.size() == 0) {
                return AdPtr();
            }
            const std::vector<Ad>::const_iterator result = std::max_element(ads.cbegin(), ads.cend(), [](const Ad &first, const Ad &second) -> bool {
                return first.auth_bid_micros && second.auth_bid_micros ? 
                       first.auth_bid_micros <second.auth_bid_micros : first.max_bid_micros < second.max_bid_micros;
            });
            return std::make_shared<Ad>(*result);
        }
        
        template<typename CampaignId>
        auto  authorize(CampaignId && campaign_id) {
            return banker.authorize(bidder_caches.budget_cache, campaign_id);
        }
        
    private:   
        SpecBidderCaches &bidder_caches;
        std::vector<GeoCampaign> geo_campaigns;
        std::vector<Ad> retrieved_cached_ads;
        AdSelectionAlg selection_alg;
        vanilla::core::Banker<vanilla::BudgetManager> banker{};
};
}

#endif /* BIDDER_SELECTOR_HPP */

