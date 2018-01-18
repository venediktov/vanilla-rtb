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
#include <tuple>
#include <type_traits>

namespace vanilla {


struct chained_selector {
template<typename T, typename Arg, typename Func , typename... Funcs> 
static typename std::enable_if< (sizeof...(Funcs) > 0)>::type
chain_function(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp, Arg&& arg, Func head, Funcs... tail) {
    auto next_arg = head(std::forward<Arg>(arg), req, imp);
    if ( next_arg ) {
        chain_function(req, imp, next_arg, tail...);
    }
}

template<typename T, typename Arg, typename Func>
static void chain_function(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp, Arg&& arg, Func terminal_func) {
    terminal_func(std::forward<Arg>(arg), req, imp);
}
};

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

        template<typename T, typename HeadArg , typename... Funcs>
        AdPtr select(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp, HeadArg&& head , Funcs... funcs) {
            AdPtr result;
            chained_selector::chain_function( req, imp, std::forward<HeadArg>(head), funcs...);
        } 

        template<typename CampaignId>
        auto  authorize(CampaignId && campaign_id) {
            return banker.authorize(bidder_caches.budget_cache, campaign_id);
        }
        
    private:   
        const BidderCaches &bidder_caches;
        std::vector<Ad> retrieved_cached_ads;
        ad_selection_algo selection_algo;
        vanilla::core::Banker<vanilla::BudgetManager> banker;
};

}

#endif /* AD_SELECTOR_HPP */

