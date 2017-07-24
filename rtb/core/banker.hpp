/* 
 * File:   banker.hpp
 * Author: Vladimir Venediktov
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on July 8, 2017, 1:42 PM
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

#pragma once
#ifndef CORE_BANKER_HPP
#define CORE_BANKER_HPP

#include <cstdint>

namespace vanilla { namespace core {
    
template<typename BudgetMgr>
class Banker
{
public :
    template<typename CampaignCache, typename CampaignId>
    auto authorize(CampaignCache && cache, CampaignId && campaign_id) {
        auto budget = cache.retrieve(std::forward<CampaignId>(campaign_id));
        return budget_mgr.authorize(budget);
    }
    private :
        BudgetMgr budget_mgr ;
};

}}
#endif /* CORE_BANKER_HPP */

