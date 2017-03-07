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

namespace vanilla {
template<typename Config = BidderConfig>
class Selector {
    public:
        Selector(const Config &config):
            config(config),
            ad_data_entity(config),
            geo_ad_data_entity(config),
            geo_data_entity(config)
        {}
            
        void load() noexcept(false) {
            auto sp = std::make_shared<std::stringstream>();
            {
                perf_timer<std::stringstream> timer(sp, "selector load");
                ad_data_entity.load();
                auto sp = std::make_shared<std::stringstream>();
                {
                   perf_timer<std::stringstream> timer(sp, "geo_ad load");
                   geo_ad_data_entity.load();
                }
                LOG(info) << sp->str() ;
                geo_data_entity.load();
                // load others
            }
            LOG(info) << sp->str() ;
        }
        
        std::shared_ptr<Ad> getAd(const openrtb::BidRequest &req, const openrtb::Impression &imp) {
            std::shared_ptr <Ad> result;
            
            std::shared_ptr<Geo> geo;
            if(!getGeo(req, geo) || !geo) {
                LOG(debug) << "No geo";
                return result;
            }
            std::vector<std::shared_ptr <GeoAd> > retrieved_cached_geo_ad;
            if(!getGeoAd(geo->geo_id, retrieved_cached_geo_ad)) {
                LOG(debug) << "No ads for geo " << geo->geo_id;
                return result;
            }
            
            std::vector<std::shared_ptr <Ad> > retrieved_cached_ads;
            if(!this->ad_data_entity.retrieve(retrieved_cached_ads, imp.banner.get().w, imp.banner.get().h)) {
                return result;
            }
            std::vector<std::shared_ptr <Ad> > intersection;
            std::set_intersection(retrieved_cached_ads.begin(), retrieved_cached_ads.end(),
                      retrieved_cached_geo_ad.begin(), retrieved_cached_geo_ad.end(),
                      std::back_inserter(intersection),
                      [](auto lhs, auto rhs) { return lhs->ad_id < rhs->ad_id; }
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
        
        bool getGeoAd(uint32_t geo_id, std::vector<std::shared_ptr <GeoAd> > &retrieved_cached_geo_ad) {
            if (!this->geo_ad_data_entity.retrieve(retrieved_cached_geo_ad, geo_id)) {
                LOG(debug) << "GeoAd retrieve failed " << geo_id;
                return false;
            }
            return true;
        }
        
        bool getGeo(const openrtb::BidRequest &req, std::shared_ptr<Geo> &geo) {
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
    private:     
        const Config &config;
        AdDataEntity<Config> ad_data_entity;
        GeoAdDataEntity<Config> geo_ad_data_entity;
        GeoDataEntity<Config> geo_data_entity;
};
}

#endif /* SELECTOR_HPP */

