#pragma once

#include "core/unicode_string.hpp"
#include "jsonv/all.hpp"
#include <boost/optional.hpp>
#include <string>
#include <list>
#include <vector>

namespace openrtb {

    enum class AuctionType : int8_t {
        FIRST_PRICE = 1,
        SECOND_PRICE = 2,
        UNDEFINED = -1
    };

    enum class AdPosition : int8_t {
        UNKNOWN = 0,
        ABOVE = 1,
        BETWEEN_DEPRECATED = 2,
        BELOW = 3,
        HEADER = 4,
        FOOTER = 5,
        SIDEBAR = 6,
        FULLSCREEN = 7,
        UNDEFINED = -1
    };

    enum class BannerAdType : int8_t {
        UNDEFINED = -1,  ///< Not explicitly specified

        XHTML_TEXT = 1,    ///< XHTML text ad. (usually mobile)
        XHTML_BANNER = 2,  ///< XHTML banner ad. (usually mobile)
        JAVASCRIPT = 3,    ///< JavaScript ad; must be valid XHTML (i.e., script tags included).
        IFRAME = 4         ///< Full iframe HTML
    };

    enum class CreativeAttribute : int8_t {
        UNDEFINED = -1,  ///< Not explicitly specified

        AUDIO_AD_AUTO_PLAY = 1,
        AUDIO_AD_USER_INITIATED = 2,
        EXPANDABLE_AUTOMATIC = 3,
        EXPANDABLE_USER_INITIATED_CLICK = 4,
        EXPANDABLE_USER_INITIATED_ROLLOVER = 5,
        IN_BANNER_VIDEO_AD_AUTO_PLAY = 6,
        IN_BANNER_VIDEO_AD_USER_INITIATED = 7,
        POP = 8,
        PROVOCATIVE_OR_SUGGESTIVE_IMAGERY = 9,
        SHAKY_FLASHING_FLICKERING_EXTREME_ANIMATION_SMILEYS = 10,
        SURVEYS = 11,
        TEXT_ONLY = 12,
        USER_INTERACTIVE = 13,
        WINDOWS_DIALOG_OR_ALERT_STYLE = 14,
        HAS_AUDIO_ON_OFF_BUTTON = 15,
        AD_CAN_BE_SKIPPED = 16
    };

    struct MimeType {
        MimeType(const std::string & type = "") : type(type)
        {}
        std::string type;
    };

    enum class FramePosition : int8_t {
        UNDEFINED = -1,  ///< Not explicitly specified

        IFRAME = 0,
        TOP_FRAME = 1
    };

    enum class ExpandableDirection : int8_t {
        UNDEFINED = -1,  ///< Not explicitly specified

        LEFT = 1,
        RIGHT = 2,
        UP = 3,
        DOWN = 4,
        FULLSCREEN = 5
    };

    enum  class ApiFramework : int8_t {
        UNDEFINED = -1,  ///< Not explicitly specified

        VPAID_1 = 1,    ///< IAB Video Player-Ad Interface Definitions V1
        VPAID_2 = 2,    ///< IAB Video Player-Ad Interface Definitions V2
        MRAID = 3,      ///< IAB Mobile Rich Media Ad Interface Definitions
        ORMMA = 4,       ///< Google Open Rich Media Mobile Advertising
        MRAID2 = 5      ///< IAB Mobile Rich Media Ad Interface Definitions V2
    };


    enum class NoBidReason : int8_t {
        UNSPECIFIED = -1,  ///< Not explicitly specified

        UNKNOWN_ERROR = 0,
        TECHNICAL_ERROR = 1,
        INVALID_REQUEST = 2,
        KNOWN_WEB_SPIDER = 3,
        SUSPECTED_NON_HUMAN_TRAFFIC = 4,
        CLOUD_DATACENTER_OR_PROXY_IP = 5,
        UNSUPPORTED_DEVICE = 6,
        BLOCKED_PUBLISHER_OR_SITE = 7,
        UNMATCHED_USER = 8
    };

    struct Banner {
        ~Banner() {}

        uint16_t w{};                     ///< Width of ad
        uint16_t h{};                     ///< Height of ad
        boost::optional<int> wmax;                  ///< max width of ad (OpenRTB 2.3)
        boost::optional<int> hmax;                  ///< max height of ad (OpenRTB 2.3)
        boost::optional<int> wmin;                  ///< min width of ad (OpenRTB 2.3)
        boost::optional<int> hmin;                  ///< min height of ad (OpenRTB 2.3)
        std::string id;                           ///< Ad ID
        AdPosition pos;                  ///< Ad position (table 6.5)
        std::vector<BannerAdType> btype;        ///< Blocked creative types (table 6.2)
        std::vector<CreativeAttribute> battr;   ///< Blocked creative attributes (table 5.3)
        std::vector<MimeType> mimes;            ///< Whitelist of content MIME types
        FramePosition topframe;          ///< Is it in the top frame (1) or an iframe (0)?
        std::vector<ExpandableDirection> expdir;///< Expandable ad directions (table 6.11)
        std::vector<ApiFramework> api;          ///< Supported APIs (table 5.6)
        jsonv::value ext;                 ///< Extensions go here, new in OpenRTB 2.3
    };


    struct Video {};
    struct PMP {};
    using ContentCategory = std::string; //struct ContentCategory {};

    struct Publisher {
        std::string id;                      ///< Unique ID representing the publisher
        std::string name; // vanilla::unicode_string name;        ///< Publisher name
        std::vector<ContentCategory> cat;    ///< Content categories     
        std::string domain; //vanilla::unicode_string domain;      ///< Domain name of publisher
        jsonv::value ext;                     ///< Extensions go here, new in OpenRTB 2.1
    };

    struct Context {
        //std::string id;                 ///< Site ID on the exchange
        std::string name; //vanilla::unicode_string name;  ///< Site name
        std::string domain; //vanilla::unicode_string domain;///< Site or app domain
        std::vector<ContentCategory> cat;        ///< IAB content categories for site/app
        std::vector<ContentCategory> sectioncat; ///< IAB content categories for subsection
        std::vector<ContentCategory> pagecat;    ///< IAB content categories for page/view
        bool privacypolicy;           ///< Has a privacy policy
        boost::optional<Publisher> publisher;    ///< Publisher of the site or app
                                                 //boost::optional<Content> content;        ///< Content of the site or app
        std::vector<std::string> keywords;                    ///< Keywords describing app
        jsonv::value ext;
    };

    struct SiteImpl {
        std::string page;          ///< URL of the page to be shown
        std::string ref;           ///< Referrer URL that got user to page
        //vanilla::unicode_string search; ///< Search string that got user to page
    };

    struct Site : Context, SiteImpl   {
        std::string id;
    };
    struct Geo {
        int type;
        std::string city;
        std::string country;
        std::string region;
        
        Geo() :
            type{}, city{}, country{}, region{}
        {}
    };
    struct App {};
    struct Device {};
    struct User {
        std::string id;
        std::string buyeruid;
        boost::optional<Geo> geo;
    };

    struct Regulations {};
    struct Native {
        std::string request;
        std::string ver;
        std::vector<ApiFramework> api;   ///< Supported APIs (table 5.6)
        std::vector<CreativeAttribute> battr;  ///< Blocked creative attributes (table 5.3)
        jsonv::value ext;
    };

    struct Impression {
        ~Impression() {}
        std::string id;                             ///< Impression ID within BR
        boost::optional<Banner> banner;           ///< If it's a banner ad
        boost::optional<Video> video;             ///< If it's a video ad
        boost::optional<Native> native;           ///< If it's a native ad
        vanilla::unicode_string displaymanager;          ///< What renders the ad
        vanilla::unicode_string displaymanagerver;        ///< What version of that thing
        bool instl{};            ///< Is it interstitial
        vanilla::unicode_string tagid;                   ///< ad tag ID for auction //TODO : utf8
        double bidfloor{};        ///< CPM bid floor
        std::string bidfloorcur;                ///< Bid floor currency
        int  secure{};           ///< Flag that requires secure https assets (1 == yes) (OpenRTB 2.2)
        std::vector<std::string> iframebuster;         ///< Supported iframe busters (for expandable/video ads)
        boost::optional<PMP> pmp;        ///< Containing any Deals eligible for the impression object
        jsonv::value ext;                   ///< Extended impression attributes
    };

    struct BidRequest {
        ~BidRequest() {}
        std::string id;                             ///< Bid request ID
        std::vector<Impression> imp;            ///< List of impressions
        boost::optional<Site> site;
        boost::optional<App> app;
        boost::optional<Device> device;
        boost::optional<User> user;
        AuctionType at;                    ///< Auction type (1=first/2=second party)
        int tmax{};                    ///< Max time avail in ms
        std::vector<std::string> wseat;              ///< Allowed buyer seats
        bool allimps{};                ///< All impressions in BR (for road-blocking)
        std::vector<std::string> cur;                ///< Allowable currencies
        std::vector<ContentCategory> bcat;        ///< Blocked advertiser categories (table 6.1)
        std::vector<vanilla::unicode_string> badv;           ///< Blocked advertiser domains
        boost::optional<Regulations> regs; ///< Regulations Object list (OpenRTB 2.2)
        jsonv::value ext;                   ///< Protocol extensions
        jsonv::value unparseable;           ///< Unparseable fields get put here
    };


//OpenRTB 2.2 Response structures

    struct Bid {
        std::string id;                       ///< Bidder's bid ID to identify bid
        std::string impid;                    ///< ID of the impression we're bidding on
        double price{};                       ///< Price to bid
        std::string adid;                     ///< Id of ad to be served if won
        std::string nurl;                     //vanilla::unicode_string nurl;                  ///< Win notice/ad markup URL
        std::string adm;                      //vanilla::unicode_string adm;                   ///< Ad markup
        std::vector<std::string> adomain;     ///< Advertiser domains
        std::string iurl;                     //vanilla::unicode_string iurl;                  ///< Image URL for content checking
        std::string cid;                      ///< Campaign ID
        std::string crid;                     ///< Creative ID
        std::vector<CreativeAttribute> attr;  ///< Creative attributes
        std::string dealid;                   ///< unique id for the deal associated with bid
                                              ///< if its in bid request, required in bid response
        int w{};                              ///< Width of ad
        int h{};                              ///< Height of ad
        jsonv::value ext;                     ///< Extended bid fields
    };

    struct SeatBid {
        std::vector<Bid> bid;  ///< Array of bid objects  (relating to imps)
        std::string seat;      ///< Seat on behalf of whom the bid is made
        int group{};            ///< If true, imps must be won as a group
        jsonv::value ext;     ///< Extension fields
    };

    struct BidResponse {
        std::string id;
        std::vector<SeatBid> seatbid;
        std::string bidid;
        std::string cur;
        std::string customdata;
        NoBidReason nbr; ///< reason for not bidding
        jsonv::value ext; //Placeholder for bidder-specific extensions to OpenRTB
    };

}
