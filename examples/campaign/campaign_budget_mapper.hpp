/* 
 * File:   campaign_budget_mapper.hpp
 * Author: Vladimir Venediktov vvenedict@gmail.com
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on June 14, 2017, 9:08 PM
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
#ifndef RTB_CAMPAIGN_BUDGET_MAPPER_HPP
#define RTB_CAMPAIGN_BUDGET_MAPPER_HPP

#include "campaign_cache.hpp"

namespace DSL {
    using namespace jsonv;

    template<typename T>
    class campaign_budget_mapper {
        using Campaign     = vanilla::CampaignBudget;
        using Metric       = typename vanilla::CampaignBudget::Metric;
        using MetricType   = typename vanilla::CampaignBudget::MetricType;
    public:
        using deserialized_type = Campaign;
        using serialized_type   = Campaign;
        using parse_error_type  = jsonv::parse_error;
        
    public:
        formats build_request() 
        {
            formats base_in = formats_builder()
                .type<Campaign>()
                .member("id", &Campaign::campaign_id)
                .member("budget", &Campaign::day_budget_limit)
                .member("spent", &Campaign::day_budget_spent)
                .member("metric", &Campaign::metric)
                .template type<Metric>()
                .member("id", &Campaign::type)
                .member("value", &Campaign::value)
                .template enum_type<MetricType>("id",
                {
                    { MetricType::UNDEFINED,  0 },
                    { MetricType::CPM,        1 },
                    { MetricType::CPC,        2 },
                    { MetricType::CPA,        3 }
                })
                .check_references(formats::defaults()) 
                ;

                return formats::compose({ base_in, formats::defaults() });
         }

         formats build_response() 
         {
            return build_request();
         }

    };

} //namespace

#endif
