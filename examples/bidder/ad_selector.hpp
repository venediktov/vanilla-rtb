/* 
 * File:   bidder_selector.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:15
 */

#ifndef BIDDER_SELECTOR_HPP
#define BIDDER_SELECTOR_HPP

#include "core/openrtb.hpp"
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
        
        AdSelector(BidderCaches<Config> &bidder_caches):
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
            if (!bidder_caches.geo_campaign_entity.retrieve(geo_campaigns, geo_id)) {
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

            if (!bidder_caches.geo_data_entity.retrieve(geo, city, country)) {
                LOG(debug) << "retrieve failed " << city << " " << country;
                return false;
            } 
            return true;
        }

        template <typename T>
        bool getCampaignAds(const std::vector<GeoCampaign> &campaigns, const openrtb::Impression<T> &imp) {
            retrieved_cached_ads.clear();
            for (auto &campaign : campaigns) {
                if (!bidder_caches.ad_data_entity.retrieve(retrieved_cached_ads, campaign.campaign_id, imp.banner.get().w, imp.banner.get().h)) {
                    continue;
                }
            }
            return retrieved_cached_ads.size() > 0;
        }
        
        AdPtr max_bid(const std::vector<Ad>& ads) {
            if(ads.size() == 0) {
                return AdPtr();
            }
            const std::vector<Ad>::const_iterator result = std::max_element(ads.cbegin(), ads.cend(), [](const Ad &first, const Ad &second) -> bool {
                return first.max_bid_micros < second.max_bid_micros;
            });
            return std::make_shared<Ad>(*result);
        }
        
    private:   
        SpecBidderCaches &bidder_caches;
        std::vector<GeoCampaign> geo_campaigns;
        std::vector<Ad> retrieved_cached_ads;
        AdSelectionAlg selection_alg;
};
}

#endif /* BIDDER_SELECTOR_HPP */

