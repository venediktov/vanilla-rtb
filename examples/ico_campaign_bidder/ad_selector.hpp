/* 
 * File:   ad_selector.hpp
 */

#ifndef AD_SELECTOR_HPP
#define AD_SELECTOR_HPP

#include "algos.hpp"
#include "referer.hpp"
#include "rtb/core/openrtb.hpp"
#include "rtb/core/banker.hpp"
#include "examples/campaign/campaign_cache.hpp"

#include <memory>
#include <algorithm>

namespace vanilla {
template<typename BidderCaches>
class ad_selector {
    public:
        using AdPtr = std::shared_ptr<Ad>;
        using ad_selection_algo = std::function<AdPtr(const std::vector<Ad>&)>;
        using self_type = ad_selector<BidderCaches>;
        
        ad_selector(const BidderCaches &bidder_caches): bidder_caches{bidder_caches} {
            retrieved_cached_ads.reserve(500);
        }
            
        self_type& with_selection_algo(const ad_selection_algo &algo) {
            selection_algo = algo;
            return *this;
        }
        
        template<typename T> 
        AdPtr select(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp) {
            AdPtr result;
//
//            Geo geo;
//            if(!getGeo(req, geo) ) {
//                LOG(debug) << "No geo";
//                return result;
//            }
//
//            if(!getGeoCampaigns(geo.geo_id)) {
//                LOG(debug) << "No campaigns for geo " << geo.geo_id;
//                return result;
//            }
//            else {
//                LOG(debug) << "Selected campaigns " << geo_campaigns.size();
//            }
//
//            if(!get_campaign_ads(geo_campaigns, imp)) {
//                LOG(debug) << "No ads for geo " << geo.geo_id;
//                return result;
//            }
//            else {
//                LOG(debug) << "selected ads " << retrieved_cached_ads.size();
//            }
//
//            if(selection_algo) {
//                return selection_alg(retrieved_cached_ads);
//            }
            return result; //algorithm::calculate_max_bid(retrieved_cached_ads);
        }
        
//        bool getGeoCampaigns(uint32_t geo_id) {
//            geo_campaigns.clear();
//            if (!bidder_caches.geo_campaign_entity.retrieve(geo_campaigns, geo_id)) {
//                LOG(debug) << "GeoAd retrieve failed " << geo_id;
//                return false;
//            }
//            return true;
//        }

//        template <typename T>
//        bool getGeo(const openrtb::BidRequest<T> &req, Geo &geo) {
//            if (!req.user) {
//                LOG(debug) << "No user";
//                return true;
//            }
//            if (!req.user.get().geo) {
//                LOG(debug) << "No user geo";
//                return true;
//            }
//
//            auto &city_view = req.user.get().geo.get().city ;
//            auto &country_view = req.user.get().geo.get().country;
//            const std::string city = boost::algorithm::to_lower_copy(std::string(city_view.data(), city_view.size()));
//            const std::string country = boost::algorithm::to_lower_copy(std::string(country_view.data(), country_view.size()));
//
//            if (!bidder_caches.geo_data_entity.retrieve(geo, city, country)) {
//                LOG(debug) << "retrieve failed " << city << " " << country;
//                return false;
//            }
//            return true;
//        }

//        template <typename T>
//        bool get_campaign_ads(const std::vector<GeoCampaign> &campaigns, const openrtb::Impression<T> &imp) {
//            retrieved_cached_ads.clear();
//            for (auto &campaign : campaigns) {
//                auto offset = retrieved_cached_ads.size();
//                if (!bidder_caches.ad_data_entity.retrieve(retrieved_cached_ads, campaign.campaign_id, imp.banner.get().w, imp.banner.get().h)) {
//                    continue;
//                }
//                auto budget_bid = authorize(campaign.campaign_id);
//                std::transform(std::begin(retrieved_cached_ads) + offset,
//                               std::end(retrieved_cached_ads),
//                               std::begin(retrieved_cached_ads) + offset, [budget_bid](Ad & ad){
//                                   ad.auth_bid_micros = std::min(budget_bid, ad.max_bid_micros);
//                                   return ad;
//                               });
//            }
//            return retrieved_cached_ads.size() > 0;
//        }

        
        template<typename CampaignId>
        auto  authorize(CampaignId && campaign_id) {
            return banker.authorize(bidder_caches.budget_cache, campaign_id);
        }
        
    private:   
        const BidderCaches &bidder_caches;
//        std::vector<GeoCampaign> geo_campaigns;
        std::vector<Ad> retrieved_cached_ads;
        ad_selection_algo selection_algo;
        vanilla::core::Banker<vanilla::BudgetManager> banker;
};

}

#endif /* AD_SELECTOR_HPP */

