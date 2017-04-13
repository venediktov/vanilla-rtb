#include "core/openrtb.hpp"
#include "encoders.hpp"
#include <vector>
#include <boost/optional.hpp>

namespace DSL {
    using namespace openrtb;
    using namespace jsonv;

    template<unsigned int Size=128>
    class GenericDSL {
    public:
        using deserialized_type = openrtb::BidRequest;
        using serialized_type = openrtb::BidResponse;
        using parse_error_type = jsonv::parse_error;

        GenericDSL() {
            
            formats base_in = formats_builder()
                .type<Banner>()
                .member("h", &Banner::h)
                .member("w", &Banner::w)
                .member("pos", &Banner::pos)
                .enum_type<AdPosition>("pos",
                {
                    { AdPosition::UNKNOWN,  0 },
                    { AdPosition::ABOVE, 1 },
                    { AdPosition::BETWEEN_DEPRECATED, 2 },
                    { AdPosition::BELOW, 3 },
                    { AdPosition::HEADER, 4 },
                    { AdPosition::FOOTER, 5 },
                    { AdPosition::SIDEBAR, 6 },
                    { AdPosition::FULLSCREEN,7 }
                })
                .type<Impression>()
                .member("id", &Impression::id)
                .member("banner", &Impression::banner)
                .member("bidfloor", &Impression::bidfloor)
                .member("bidfloorcur", &Impression::bidfloorcur)
                .type<User>()
                    .member("id", &User::id)
                    .member("buyeruid", &User::buyeruid)
                    .member("geo", &User::geo)
                .type<Geo>()
                    .member("city", &Geo::city)
                    .member("country", &Geo::country)
                .type<Site>()
                .member("id", &Site::id)
                .type<Publisher>()
                .type<BidRequest>()
                .member("id", &BidRequest::id)
                .member("imp", &BidRequest::imp)
                .member("user", &BidRequest::user)
                .member("site", &BidRequest::site)
                .encode_if([](const jsonv::serialization_context&, const boost::optional<Site>& x) {return bool(x);})
                .template register_container<std::vector<Impression>>()
                .template register_optional<boost::optional<Banner>>()
                .template register_optional<boost::optional<Site>>()
                .template register_optional<boost::optional<Publisher>>()
                .template register_optional<boost::optional<User>>()
                .template register_optional<boost::optional<Geo>>()
                .template register_container<std::vector<std::string>>()
                .template register_container<std::vector<int>>()
                .check_references(formats::defaults())
                ;

            request_fmt_ = formats::compose({ base_in, formats::defaults() });

            formats base_out = formats_builder()
                .type<Bid>()
                .member("id", &Bid::id)
                .member("impid", &Bid::impid)
                .member("price", &Bid::price)
                .member("adid", &Bid::adid)
                .member("nurl", &Bid::nurl)
                .member("adm", &Bid::adm)
                .member("adomain", &Bid::adomain)
                .member("iurl", &Bid::iurl)
                .member("cid", &Bid::cid)
                .member("crid", &Bid::crid)
                .member("attr", &Bid::attr)
                .enum_type<CreativeAttribute>("attr",
                {
                    { CreativeAttribute::UNDEFINED,  -1},
                    { CreativeAttribute::AUDIO_AD_AUTO_PLAY, 1},
                    { CreativeAttribute::AUDIO_AD_USER_INITIATED, 2},
                    { CreativeAttribute::EXPANDABLE_AUTOMATIC, 3},
                    { CreativeAttribute::EXPANDABLE_USER_INITIATED_CLICK, 4},
                    { CreativeAttribute::EXPANDABLE_USER_INITIATED_ROLLOVER, 5},
                    { CreativeAttribute::IN_BANNER_VIDEO_AD_AUTO_PLAY, 6},
                    { CreativeAttribute::IN_BANNER_VIDEO_AD_USER_INITIATED, 7},
                    { CreativeAttribute::POP, 8},
                    { CreativeAttribute::PROVOCATIVE_OR_SUGGESTIVE_IMAGERY, 9},
                    { CreativeAttribute::SHAKY_FLASHING_FLICKERING_EXTREME_ANIMATION_SMILEYS, 10},
                    { CreativeAttribute::SURVEYS, 11},
                    { CreativeAttribute::TEXT_ONLY, 12},
                    { CreativeAttribute::USER_INTERACTIVE, 13},
                    { CreativeAttribute::WINDOWS_DIALOG_OR_ALERT_STYLE, 14},
                    { CreativeAttribute::HAS_AUDIO_ON_OFF_BUTTON, 15},
                    { CreativeAttribute::AD_CAN_BE_SKIPPED, 16}
                })
                .type<SeatBid>()
                .member("bid", &SeatBid::bid)
                .member("seat", &SeatBid::seat)
                .member("group", &SeatBid::group)
                .member("ext", &SeatBid::ext)
                .type<BidResponse>()
                .member("id", &BidResponse::id)
                .member("seatbid", &BidResponse::seatbid)
                .member("bidid", &BidResponse::bidid)
                .member("cur", &BidResponse::cur)
                .member("customdata", &BidResponse::customdata)
                .member("nbr", &BidResponse::nbr)
                .member("ext", &BidResponse::ext)
                .enum_type<NoBidReason>("nbr",
                {
                    { NoBidReason::UNKNOWN_ERROR,  0 },
                    { NoBidReason::TECHNICAL_ERROR, 1 },
                    { NoBidReason::INVALID_REQUEST, 2 },
                    { NoBidReason::KNOWN_WEB_SPIDER, 3 },
                    { NoBidReason::SUSPECTED_NON_HUMAN_TRAFFIC, 4 },
                    { NoBidReason::CLOUD_DATACENTER_OR_PROXY_IP, 5 },
                    { NoBidReason::UNSUPPORTED_DEVICE, 6 },
                    { NoBidReason::BLOCKED_PUBLISHER_OR_SITE,7 },
                    { NoBidReason::UNMATCHED_USER,8 }
                })
                .register_container<std::vector<SeatBid>>()
                .register_container<std::vector<Bid>>()
                .register_container<std::vector<CreativeAttribute>>()
                .register_container<std::vector<std::string>>()
                .check_references(formats::defaults())
                ;

            response_fmt_ = formats::compose({ base_out, formats::defaults() });

        }

        template<typename string_view_type>
        deserialized_type extract_request(const string_view_type & bid_request) {
            //auto encoded = parse(bid_request);
            jsmn_parser parser;
            jsmntok_t t[Size];
            jsonv::value encoded;
            jsmn_init(&parser);
            auto r = jsmn_parse(&parser, bid_request.c_str(), bid_request.length(), t, sizeof(t)/sizeof(t[0]));
            if (r < 0) {
                throw std::runtime_error("DSL parse exception");
            }
            encoders::encode(bid_request.c_str(), &t[0], parser.toknext, encoded);
            return extract<openrtb::BidRequest>(encoded, request_fmt_);
        }

        jsonv::value create_response(const serialized_type & bid_response) {
            return to_json(bid_response, response_fmt_);
        }

    private:
        formats request_fmt_;
        formats response_fmt_;
    };
}
/*

 */