/* 
 * File:   ad_selector.hpp
 * * Author: Vladimir Venediktov
 * * Author: Arseny Bushev
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on January 8, 2018, 1:42 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef AD_SELECTOR_HPP
#define AD_SELECTOR_HPP

#include "algos.hpp"
#include "banker.hpp"
#include "tagged_tuple.hpp"
#include "common/type_algo.hpp"
#include "core/algos.hpp"
#include <memory>
#include <type_traits>


namespace vanilla {

struct chained_selector {
template<typename BidRequest, typename Impression, typename Arg, typename Func , typename... Funcs>
static typename std::enable_if< (sizeof...(Funcs) > 0), typename terminal_function<Func,Funcs...>::template return_type<Arg,BidRequest,Impression> >::type
chain_function(BidRequest&& req, Impression&& imp, Arg&& arg, Func head, Funcs... tail) {
    //using return_type = typename vanilla::nth_function<sizeof...(Funcs)-1,Funcs...>::template return_type<Arg,BidRequest<T>,Impression<T>>::type;
    using return_type = typename terminal_function<Func,Funcs...>::template return_type<Arg,BidRequest,Impression>;
    auto next_arg = head(std::forward<Arg>(arg), std::forward<decltype(req)>(req), std::forward<decltype(imp)>(imp));
    if ( next_arg ) {
        return chain_function(std::forward<BidRequest>(req), std::forward<Impression>(imp), next_arg, tail...);
    }
    return return_type();
}

template<typename BidRequest, typename Impression, typename Arg, typename Func>
static auto chain_function(BidRequest&& req, Impression&& imp, Arg&& arg, Func terminal_func) {
    return terminal_func(std::forward<Arg>(arg), std::forward<BidRequest>(req), std::forward<Impression>(imp));
}
};

template<typename BudgetManager>
class ad_selector {
    public:
//        using AdPtr = std::shared_ptr<Ad>;
        using ad_selection_algo = std::function<Ad(const std::vector<Ad>&)>;
        using self_type = ad_selector<BudgetManager>;

//        self_type& with_selection_algo(const ad_selection_algo &algo) {
//            selection_algo = algo;
//            return *this;
//        }

        template<typename BidRequest, typename Impression, typename HeadArg , typename... Funcs>
        auto select(BidRequest&& req, Impression&& imp, HeadArg&& head , Funcs... funcs) {
            auto ads = chained_selector::chain_function( std::forward<BidRequest>(req), std::forward<Impression>(imp), std::forward<HeadArg>(head), funcs...);
//            if(selection_alg) {
//                return selection_alg(ads);
//            }
            return algorithm::calculate_max_bid(ads);
        }

        template<typename BudgetCache , typename CampaignId>
        auto  authorize(BudgetCache & cache , CampaignId && campaign_id) {
            return banker.authorize(cache, std::forward<CampaignId>(campaign_id));
        }
        
    private:
//        ad_selection_algo selection_algo;
        vanilla::core::Banker<BudgetManager> banker;
};

}

#endif /* AD_SELECTOR_HPP */

