#ifndef __RTB_DSL_CAMPAIGN_DSL_HPP_
#define __RTB_DSL_CAMPAIGN_DSL_HPP_

#include "jsonv/all.hpp"

namespace DSL {
    using namespace jsonv;

    template<typename Campaign>
    class CampaignDSL {
    public:
        using deserialized_type = Campaign;
        using serialized_type = Campaign;
        using parse_error_type = jsonv::parse_error;

 
        CampaignDSL() {
            formats base_in = formats_builder()
                .type<Campaign>()
                .member("id", &Campaign::campain_id)
                .member("budget", &Campaign::day_budget_limit)
                .member("spent", &Campaign::day_budget_spent)
                .member("cpm", &Campaign::day_show_limit)
                .member("cpc", &Campaign::day_click_limit)
                .check_references(formats::defaults())
                ;
            request_fmt_ = formats::compose({ base_in, formats::defaults() });
            //formats base_out = base_in;
            response_fmt_ = formats::compose({ base_in, formats::defaults() });

        }

        Campaign extract_request(const std::string & campaign_request) {
            auto encoded = parse(campaign_request);
            return extract<Campaign>(encoded, request_fmt_);
        }

        jsonv::value create_response(const Campaign & campaign) {
            return to_json(campaign, response_fmt_);
        }

    private:
        formats request_fmt_;
        formats response_fmt_;
    };
}

#endif