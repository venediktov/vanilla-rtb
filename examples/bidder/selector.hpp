/* 
 * File:   selector.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:15
 */

#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "core/openrtb.hpp"
#include "ad.hpp"
#include "geo_ad.hpp"
#include "geo.hpp"
#include "geo_campaign.hpp"
#include "campaign_data.hpp"

namespace vanilla {
template<typename Config = BidderConfig>
class Selector {
    public:
        using ad_retrieve_type = std::shared_ptr<Ad>;
        struct intersc_cmp {
            bool operator()(const uint64_t &l, const ad_retrieve_type &r) const {
                return l < r->ad_id;
            }
            bool operator()(const ad_retrieve_type &l, const uint64_t &r) const {
                return l->ad_id < r;
            }
        };
        Selector(const Config &config):
            config(config),
            ad_data_entity(config),
            geo_data_entity(config), 
            geo_campaign_entity(config),
            campaign_data_entity(config)
        {}
        
        void load() noexcept(false) {
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "\nselector load");
                {
                    perf_timer<std::stringstream> timer(sp, "\nad load");
                    ad_data_entity.load();
                }
                {
                   perf_timer<std::stringstream> timer(sp, "\ngeo_data load");
                    geo_data_entity.load();
                }
                {
                   perf_timer<std::stringstream> timer(sp, "\ngeo_campaign load");
                   geo_campaign_entity.load();
                }
                {
                   perf_timer<std::stringstream> timer(sp, "\ncampaign data load");
                   campaign_data_entity.load();
                }
                // load others
            }
            LOG(info) << sp->str() ;
        }
       
        template<typename T> 
        std::shared_ptr<Ad> getAd(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp) {
            std::shared_ptr <Ad> result;
            
            std::shared_ptr<Geo> geo;
            if(!getGeo(req, geo) || !geo) {
                LOG(debug) << "No geo";
                return result;
            }
            GeoCampaigns geo_campaigns;
            if(!getGeoCampaigns(geo->geo_id, geo_campaigns)) {
                LOG(debug) << "No campaigns for geo " << geo->geo_id;
                return result;
            }
            else {
                LOG(debug) << "Selected campaigns " << geo_campaigns.campaign_ids.size();
            }
            std::set<uint64_t> ads;
            if(!getCampaignAds(geo_campaigns.campaign_ids, ads)) {
                LOG(debug) << "No ads for geo " << geo->geo_id;
                return result;
            }
            else {
                LOG(debug) << "selected ads " << ads.size();
            }
            return result;
            std::vector<ad_retrieve_type> retrieved_cached_ads;
            if(!this->ad_data_entity.retrieve(retrieved_cached_ads, imp.banner.get().w, imp.banner.get().h)) {
                return result;
            }
            LOG(debug) << "size (" << imp.banner.get().w << "x"  << imp.banner.get().h << ") ads count " << retrieved_cached_ads.size();
            
            std::vector<ad_retrieve_type> intersection;
            std::set_intersection(retrieved_cached_ads.begin(), retrieved_cached_ads.end(),
                      ads.begin(), ads.end(),
                      std::back_inserter(intersection),
                      intersc_cmp()
                      //[](auto lhs, auto rhs) { return lhs->ad_id < rhs->ad_id; }
            );
            // Sort by cicros
            // TODO make ability to make custom algorithm
            std::sort(intersection.begin(), intersection.end(),
                    [](const std::shared_ptr<Ad> &first, const std::shared_ptr<Ad> &second) -> bool {
                        return first->max_bid_micros > second->max_bid_micros;
            });
            LOG(debug) << "intersecion size " << intersection.size();
            if(intersection.size()) {
                result = intersection[0];
            }
            return result;
        }
        
        bool getGeoCampaigns(uint32_t geo_id, GeoCampaigns &retrieved_cached_geo_campaign) {
            if (!this->geo_campaign_entity.retrieve(retrieved_cached_geo_campaign, geo_id)) {
                LOG(debug) << "GeoAd retrieve failed " << geo_id;
                return false;
            }
            return true;
        }
        
        template<typename T> 
        bool getGeo(const openrtb::BidRequest<T> &req, std::shared_ptr<Geo> &geo) {
            std::vector<std::shared_ptr <Geo> > retrieved_cached_geo;
            if (!req.user) {
                LOG(debug) << "No user";
                return true;
            }
            if (!req.user.get().geo) {
                LOG(debug) << "No user geo";
                return true;
            }
            
            const std::string city = boost::algorithm::to_lower_copy(req.user.get().geo.get().city);
            const std::string country = boost::algorithm::to_lower_copy(req.user.get().geo.get().country);

            if (!this->geo_data_entity.retrieve(retrieved_cached_geo, city, country)) {
                LOG(debug) << "retrieve failed " << city << " " << country;
                return false;
            }
            if (!retrieved_cached_geo.size()) {
                return false;
            }
            geo = retrieved_cached_geo[0];
            return true;
        }
        bool getCampaignAds(const GeoCampaigns::collection_type &campaigns, std::set<uint64_t> &ads) {
            CampaignData campaign_data;
            LOG(debug) << "select ads from  " << campaigns.size() << " campaigns";
            for(auto &campaign : campaigns) {
                LOG(debug) << "search campaign " << campaign;
                campaign_data.clear();
                if(!this->campaign_data_entity.retrieve(campaign_data, campaign)) {
                    continue;
                }
                LOG(debug) << "ads in campaign " << campaign_data.ad_ids.size();
                std::for_each(campaign_data.ad_ids.begin(), campaign_data.ad_ids.end(), [&](auto &v) {
                    ads.insert(v);
                });
            }
        }
    private:     
        
        const Config &config;
        AdDataEntity<Config> ad_data_entity;
        GeoDataEntity<Config> geo_data_entity;
        GeoCampaignEntity<Config> geo_campaign_entity;
        CampaignDataEntity<Config> campaign_data_entity;
};
}

#endif /* SELECTOR_HPP */

