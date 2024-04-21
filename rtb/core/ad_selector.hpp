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
#include <functional>


namespace vanilla {

namespace detail {

template <typename BidRequest, typename Impression, typename Arg, typename Func,
          typename... Funcs>
static auto chain_function(BidRequest &&req, Impression &&imp, Arg &&arg,
                           Func head, Funcs... tail) {
  if constexpr (sizeof...(tail) == 0) {
    return head(std::forward<Arg>(arg), std::forward<BidRequest>(req),
                std::forward<Impression>(imp));
  } else if constexpr (sizeof...(tail) > 0) {
    return chain_function(
        std::forward<BidRequest>(req), std::forward<Impression>(imp),
        head(std::forward<Arg>(arg), std::forward<BidRequest>(req),
             std::forward<Impression>(imp)),
        tail...);
  }
}

} // namespace detail

template <typename T>
concept is_vanilla_standard_ad = requires(T value) {
    value.auth_bid_micros;
    value.max_bid_micros;
};

template<typename BudgetManager, typename Ad>
class ad_selector {
    public:
        using ad_selection_func = std::function<std::unique_ptr<Ad> (const std::vector<Ad>&)>;
        using self_type = ad_selector<BudgetManager, Ad>;

        self_type& with_selection_algo(const ad_selection_func &algo) {
            selection_algo = algo;
            return *this;
        }

        template<typename BidRequest, typename Impression, typename HeadArg , typename... Funcs>
        auto select(BidRequest&& req, Impression&& imp, HeadArg&& head, Funcs... funcs) {
            auto ads = detail::chain_function( std::forward<BidRequest>(req), std::forward<Impression>(imp), std::forward<HeadArg>(head), funcs...);
            if (selection_algo) {
                return selection_algo(ads);
            } else {
                if constexpr ( is_vanilla_standard_ad<Ad>) {
                    return algorithm::calculate_max_bid(ads);
                } else {
                    // TODO: add compile time warning for using dummy algo
                    return std::unique_ptr<Ad>{};
                }
            }
        }

        template<typename BudgetCache , typename CampaignId>
        auto  authorize(BudgetCache & cache , CampaignId && campaign_id) {
            return banker.authorize(cache, std::forward<CampaignId>(campaign_id));
        }
        
    private:
        ad_selection_func selection_algo;
        vanilla::core::Banker<BudgetManager> banker;
};

}

#endif /* AD_SELECTOR_HPP */

