#ifndef __RTB_DSL_CAMPAIGN_DSL_HPP_
#define __RTB_DSL_CAMPAIGN_DSL_HPP_

#include "jsonv/all.hpp"

namespace DSL {
    using namespace jsonv;

    template<typename Mapper>
    class CampaignDSL : Mapper {
    public:
        using deserialized_type = typename Mapper::deserialized_type;
        using serialized_type   = typename Mapper::serialized_type;
        using parse_error_type  = typename Mapper::parse_error_type;

 
        CampaignDSL() {
            request_fmt_  = this->build_request();
            response_fmt_ = this->build_response();
        }

        deserialized_type extract_request(const std::string & campaign_entity) {
            auto encoded = parse(campaign_entity);
            return extract<deserialized_type>(encoded, request_fmt_);
        }

        auto create_response(const serialized_type & campaign_response) {
            return to_json(campaign_response, response_fmt_);
        }

    private:
        formats request_fmt_;
        formats response_fmt_;
    };
}

#endif
