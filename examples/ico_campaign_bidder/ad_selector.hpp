/* 
 * File:   ad_selector.hpp
 */

#ifndef AD_SELECTOR_HPP
#define AD_SELECTOR_HPP

#include "algos.hpp"
#include "rtb/core/banker.hpp"
#include <memory>


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

template<typename BudgetManager>
class ad_selector {
    public:
        using AdPtr = std::shared_ptr<Ad>;
        using ad_selection_algo = std::function<AdPtr(const std::vector<Ad>&)>;
        using self_type = ad_selector<BudgetManager>;

        self_type& with_selection_algo(const ad_selection_algo &algo) {
            selection_algo = algo;
            return *this;
        }

        template<typename T, typename HeadArg , typename... Funcs>
        AdPtr select(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp, HeadArg&& head , Funcs... funcs) {
            AdPtr result;
            chained_selector::chain_function( req, imp, std::forward<HeadArg>(head), funcs...);
        } 

        template<typename BudgetCache , typename CampaignId>
        auto  authorize(BudgetCache & cache , CampaignId && campaign_id) {
            return banker.authorize(cache, std::forward<CampaignId>(campaign_id));
        }
        
    private:
        ad_selection_algo selection_algo;
        vanilla::core::Banker<BudgetManager> banker;
};

}

#endif /* AD_SELECTOR_HPP */

