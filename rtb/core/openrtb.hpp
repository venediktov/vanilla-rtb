#pragma once

#include "unicode_string.hpp"

#include <boost/optional.hpp>
#include <string>
#include <list>
#include <vector>
#include <cstdint>

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

    template<typename T>
    struct MimeType {
        MimeType(const T & type = "") : type(type)
        {}
        T type;
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

    template<typename T>
    struct Format {
        int w{};                            ///< Width in device independent pixels (DIPS)
        int h{};                            ///< Height in device independent pixels (DIPS).
        int wratio{};                       ///< Relative width when expressing size as a ratio.
        int hratio{};                       ///< Relative height when expressing size as a ratio.
        int wmin{};                         ///< The minimum width in device independent pixels (DIPS) at
                                            ///< which the ad will be displayed the size is expressed as a ratio.
        T ext;                              ///< Placeholder for exchange-specific extensions to OpenRTB.
    };

    template<typename T>
    struct Banner {
        ~Banner() {}

        std::vector<Format<T>> format;      ///< Array of format objects (Section 3.2.10) representing the
                                            ///< banner sizes permitted. If none are specified, then use of the
                                            ///< h and w attributes is highly recommended.
        uint16_t w{};                     ///< Width of ad
        uint16_t h{};                     ///< Height of ad
        boost::optional<int> wmax;                  ///< max width of ad (OpenRTB 2.3)
        boost::optional<int> hmax;                  ///< max height of ad (OpenRTB 2.3)
        boost::optional<int> wmin;                  ///< min width of ad (OpenRTB 2.3)
        boost::optional<int> hmin;                  ///< min height of ad (OpenRTB 2.3)
        T id;                           ///< Ad ID
        AdPosition pos;                  ///< Ad position (table 6.5)
        std::vector<BannerAdType> btype;        ///< Blocked creative types (table 6.2)
        std::vector<CreativeAttribute> battr;   ///< Blocked creative attributes (table 5.3)
        std::vector<MimeType<T>> mimes;            ///< Whitelist of content MIME types
        FramePosition topframe;          ///< Is it in the top frame (1) or an iframe (0)?
        std::vector<ExpandableDirection> expdir;///< Expandable ad directions (table 6.11)
        std::vector<ApiFramework> api;          ///< Supported APIs (table 5.6)
        T ext; //jsonv::value ext;                 ///< Extensions go here, new in OpenRTB 2.3
    };

    enum class Protocol: uint8_t {
        UNKNOWN = 0,
        VAST_1_0,
        VAST_2_0,
        VAST_3_0,
        VAST_1_0_WRAPPER,
        VAST_2_0_WRAPPER,
        VAST_3_0_WRAPPER,
        VAST_4_0,
        VAST_4_0_WRAPPER,
        DAAST_1_0,
        DAAST_1_0_WRAPPER
    };

    enum class VideoPlacement {
        UNKNOWN = 0,
        IN_STREAM, /// In-Stream
                   /// Played before, during or after the streaming video content that the consumer has requested
                   /// (e.g., Pre-roll, Mid-roll, Post-roll).
                   /// OpenRTB API Specification Version 2.5 IAB Technology Lab
                   ///www.iab.com/openrtb Page 48
        IN_BANNER, /// In-Banner
                   /// Exists within a web banner that leverages the banner space to deliver a video experience as
                   /// opposed to another static or rich media format. The format relies on the existence of display
                   /// ad inventory on the page for its delivery.
        IN_ARTICLE,/// In-Article
                   /// Loads and plays dynamically between paragraphs of editorial content; existing as a standalone
                   /// branded message.
        IN_FEED,   /// In-Feed - Found in content, social, or product feeds.
        FLOATING   /// Interstitial/Slider/Floating
                   /// Covers the entire or a portion of screen area, but is always on screen while displayed (i.e.
                   /// cannot be scrolled out of view). Note that a full-screen interstitial (e.g., in mobile) can be
                   /// distinguished from a floating/slider unit by the imp.instl field
    };

    enum class VideoLinearity {

    };

    enum class PlaybackMethod: uint8_t {
        UNKNOWN = 0,
        ON_PAGE_SOUND_ON,       /// Initiates on Page Load with Sound On
        ON_PAGE_SOUND_OFF,      /// Initiates on Page Load with Sound Off by Default
        ON_CLICK_SOUND_ON,      /// Initiates on Click with Sound On
        ON_MOVER_SOUND_ON,      /// Initiates on Mouse-Over with Sound On
        ON_VIEWPORT_SOUND_ON,   /// Initiates on Entering Viewport with Sound On
        ON_VIEWPORT_SOUND_OFF   /// Initiates on Entering Viewport with Sound Off by Default
    };

    enum class PlaybackCessationModes: uint8_t{
        UNKNOWN = 0,
        COMPLEETION, // On Video Completion or when Terminated by User
        VIEWPORT, // On Leaving Viewport or when Terminated by User
        FLOATING // On Leaving Viewport Continues as a Floating/Slider Unit until Video Completion or when Terminated by User
    };

    enum class DeliveryMethod {
        UNKNOWN = 0,
        STREAMING,
        PROGRESSIVE,
        DOWNLOAD
    };

    enum class CompanionType {
        UNKNOWN = 0,
        STATIC,
        HTML,
        IFRAME
    };

    template<typename T>
    struct Video {
        std::vector<MimeType<T>> mimes;                 ///< Whitelist of content MIME types
        uint32_t minduration{};                         /// Minimum video ad duration in seconds.
        uint32_t maxduration{};                         /// Maximum video ad duration in seconds.
        std::vector<Protocol> protocols;                /// Array of supported video protocols.
        Protocol protocol;                              /// Deprecated  in favor of protocols
        boost::optional<uint16_t> w;                    /// Width of the video player in device independent pixels (DIPS).
        boost::optional<uint16_t> h;                    /// Height of the video player in device independent pixels (DIPS).
        boost::optional<uint32_t> startdelay;           /// Indicates the start delay in seconds for pre-roll, mid-roll, or
                                                        /// post-roll ad placements.
        VideoPlacement placement;                       /// Placement type for the impression.
        VideoLinearity linearity;                       /// Indicates if the impression must be linear, nonlinear, etc. If
                                                        /// none specified, assume all are allowed.
        boost::optional<uint8_t> skip;                  /// Indicates if the player will allow the video to be skipped,
        boost::optional<uint32_t> skipmin;              /// Videos of total duration greater than this number of seconds
                                                        /// can be skippable; only applicable if the ad is skippable.
        boost::optional<uint32_t> skipafter;            /// Number of seconds a video must play before skipping is
                                                        /// enabled; only applicable if the ad is skippable.
        boost::optional<uint16_t> sequence;             /// If multiple ad impressions are offered in the same bid request,
                                                        /// the sequence number will allow for the coordinated delivery
                                                        /// of multiple creatives.
        std::vector<CreativeAttribute> battr;           /// Blocked creative attributes.

        boost::optional<int> maxextended;               /// Maximum extended ad duration if extension is allowed. If
                                                        ///blank or 0, extension is not allowed. If -1, extension is
                                                        /// allowed, and there is no time limit imposed. If greater than 0,
                                                        /// then the value represents the number of seconds of extended
                                                        /// play supported beyond the maxduration value.
        boost::optional<uint32_t> minbitrate;           /// Minimum bit rate in Kbps.
        boost::optional<uint32_t> maxbitrate;           /// Maximum bit rate in Kbps.
        boost::optional<uint8_t> boxingallowed{1};/// Indicates if letter-boxing of 4:3 content into a 16:9 window is
                                                        /// allowed, where 0 = no, 1 = yes.
        std::vector<PlaybackMethod> playbackmethod;     /// Playback methods that may be in use. If none are specified,
                                                        /// any method may be used. Refer to List 5.10. Only one
                                                        /// method is typically used in practice. As a result, this array may
                                                        /// be converted to an integer in a future version of the
                                                        /// specification. It is strongly advised to use only the first
                                                        /// element of this array in preparation for this change.
        PlaybackCessationModes playbackend;             /// The event that causes playback to end.
        std::vector<DeliveryMethod> delivery;           /// Supported delivery methods (e.g., streaming, progressive). If
                                                        /// none specified, assume all are supported.
        AdPosition pos;                                 /// Ad position on screen.
        std::vector<CompanionType>  companiontype;      /// Supported VAST companion ad types.
                                                        /// Recommended if companion Banner objects are included via
                                                        /// the companionad array. If one of these banners will be
                                                        /// rendered as an end-card, this can be specified using the vcm
                                                        /// attribute with the particular banner
         T ext; //jsonv::value ext;                               /// Extensions go here, new in OpenRTB 2.3
    };
    struct PMP {};
    template<typename T>
    using ContentCategory = T; //struct ContentCategory {};

    template<typename T>
    struct Publisher {
        T id;                      ///< Unique ID representing the publisher
        T name; // vanilla::unicode_string name;        ///< Publisher name
        std::vector<ContentCategory<T>> cat;    ///< Content categories
        T domain; //vanilla::unicode_string domain;      ///< Domain name of publisher
        T ext; //jsonv::value ext;                     ///< Extensions go here, new in OpenRTB 2.1
    };

    template<typename T>
    struct Content {
        T id;                                           ///< ID uniquely identifying the content
        boost::optional<int> episode;                   ///< Episode number
        T title;                                        ///< Content title
        T url;                                          ///< URL of the content, for buy-side contextualization or review.
        std::vector<ContentCategory<T>> cat;            ///< Array of IAB content categories
    };

    template<typename T>
    struct Context {
        T id;
        T name;
        T domain;
        std::vector<ContentCategory<T>> cat;        ///< IAB content categories for site/app
        std::vector<ContentCategory<T>> sectioncat; ///< IAB content categories for subsection
        std::vector<ContentCategory<T>> pagecat;    ///< IAB content categories for page/view
        boost::optional<int> privacypolicy{};       ///< Has a privacy policy
        boost::optional<Publisher<T>> publisher;    ///< Publisher of the site or app
        boost::optional<Content<T>> content;        ///< Content of the site or app
        std::vector<T> keywords;                    ///< Keywords describing app
        T ext; //jsonv::value ext;
    };

    template<typename T>
    struct Site : Context<T>   {
        T page;
        T ref;
        T search;
        boost::optional<int> mobile;
    };

    template<typename T>
    struct App : Context<T> {
        T bundle;
        T storeurl;
        T ver;
        boost::optional<int> paid;
    };

    enum class GeoType : int8_t {
        UNDEFINED = -1,  ///< Not explicitly specified

        GPS = 1,        ///< GPS/Location Services
        IP = 2,         ///< IP Address
        USER = 3        ///< User provided (e.g., registration data)
    };

    enum class DeviceType : int8_t {
        UNDEFINED = -1,       ///< Not explicitly specified

        MOBILE_TABLET = 1,    ///< Version 2.0
        PC = 2,               ///< Version 2.0
        CONNECTED_TV = 3,     ///< Version 2.0
        PHONE = 4,            ///< Phone New for Version 2.2
        TABLET = 5,           ///< Tablet New for Version 2.2
        CONNECTED_DEVICE = 6, ///< Connected Device New for Version 2.2
        SET_TOBOX = 7         ///< Set Top Box New for Version 2.2
    };

    template<typename T>
    struct Geo {
        boost::optional<float> lat{};       ///< Latitude from -90.0 to +90.0, where negative is south.
        boost::optional<float> lon{};       ///< Longitude from -180.0 to +180.0, where negative is west.
        GeoType type{GeoType::UNDEFINED};   ///< Source of location data; recommended when passing lat/lon
        int utcoffset{};                    ///< Local time as the number +/- of minutes from UTC.
        T city;                             ///< City using United Nations Code for Trade & Transport
                                            ///<        Locations. See Appendix A for a link to the codes. */
        T country;                          ///< Country code using ISO-3166-1-alpha-3
        T region;                           ///< Region code using ISO-3166-2; 2-letter state code if USA.
        T regionfips104;                    ///< Region of a country using FIPS 10-4 notation. While OpenRTB
                                            ///<                  supports this attribute, it has been withdrawn by NIST in 2008. */
        T metro;                            ///< Google metro code; similar to but not exactly Nielsen DMAs.
                                            ///< See Appendix A for a link to the codes
        T zip;                              ///< Zip or postal code
        T ext; //jsonv::value ext;          ///< Placeholder for exchange-specific extensions to OpenRTB.
    };

    template<typename T>
    struct Device {
        T ua;
        //boost::optional<SUserAgent> sua;
        boost::optional<Geo<T>> geo;
        boost::optional<int> dnt;           ///<  Standard “Do Not Track” flag as set in the header by the browser,
                                            ///<    where 0 = tracking is unrestricted, 1 = do not track. */
        boost::optional<int> lmt;           ///< “Limit Ad Tracking” signal commercially endorsed (e.g., iOS,
                                            ///< Android), where 0 = tracking is unrestricted, 1 = tracking must
                                            ///< be limited per commercial guideline
        T ip;
        T ipv6;
        DeviceType devicetype{DeviceType::UNDEFINED};
        T make;
        T model;
        T os;
        T osv;                              ///< Device operating system version
        T hwv;                              ///< Hardware version of the device (e.g., “5S” for iPhone 5S).
        boost::optional<int> h;             ///< Physical height of the screen in pixels.
        boost::optional<int> w;             ///< Physical width of the screen in pixels.
        boost::optional<int> ppi;           ///< Screen size as pixels per linear inch.
        boost::optional<int> js;            ///< Support for JavaScript, where 0 = no, 1 = yes.
        boost::optional<int> geofetch;      ///< Indicates if the geolocation API will be available to JavaScript
                                            ///< code running in the banner, where 0 = no, 1 = yes.
        T flashver;                         ///< Version of Flash supported by the browser.
        T language;                         ///< Browser language using ISO-639-1-alpha-2.
        T carrier;                          ///< Carrier or ISP (e.g., “VERIZON”) using exchange curated string
                                            ///< names which should be published to bidders a priori.
        T mccmnc;                           ///< Mobile carrier as the concatenated MCC-MNC code
        boost::optional<int> connectiontype;///< Network connection type.
        T ifa;                              ///< ID sanctioned for advertiser use in the clear
        T didsha1;                          ///< Hardware device ID
        T didmd5;                           ///< Hardware device ID (e.g., IMEI); hashed via MD5.
        T dpidsha1;                         ///< Platform device ID (e.g., Android ID); hashed via SHA1.
        T dpidmd5;                          ///< Platform device ID (e.g., Android ID); hashed via MD5.
        T macsha1;                          ///< MAC address of the device; hashed via SHA1
        T macmd5;                           ///< MAC address of the device; hashed via MD5.
        T ext;
    };

    template<typename T>
    struct UserDataSegment {
        T id;                                ///< ID of the data segment specific to the data provider.
        T name;                              ///< Name of the data segment specific to the data provider.
        T value;                             ///< String representation of the data segment value.
        T ext; //jsonv::value ext;           ///< Placeholder for exchange-specific extensions to OpenRTB.
    };
    template<typename T>
    struct UserData {
        T id;                                 ///< Exchange-specific ID for the data provider
        T name;                               ///< Exchange-specific name for the data provider
        std::vector<UserDataSegment<T>> segment;   ///< Array of Segment objects that contain the actual data values.
        T ext; //jsonv::value ext;             ///< Placeholder for exchange-specific extensions to OpenRTB.
    };
    template<typename T>
    struct User {
        T id;             ///< Exchange-specific ID for the user.
        T buyeruid;       ///< Buyer-specific ID for the user as mapped by the exchange for the buyer
        boost::optional<int> yob;          ///< Year of birth as a 4-digit integer
        T gender;         ///< Gender, where “M” = male, “F” = female, “O” = known to be other (i.e., omitted is unknown).
        T keywords;       ///< Comma separated list of keywords, interests, or intent
        T customdata;     ///< Optional feature to pass bidder data that was set in the
                                           ///  exchange’s cookie. The string must be in base85 cookie safe
                                            ///  characters and be in any format. Proper JSON encoding must
                                            ///  be used to include “escaped” quotation marks.
        boost::optional<Geo<T>> geo;
        std::vector<UserData<T>> data;     ///< Additional user data
        T ext;             //jsonv::value ext;           ///< Placeholder for exchange-specific extensions to OpenRTB.

    };

    template<typename T>
    struct Regulations {
        int coppa{};
    };

    template<typename T>
    struct Native {
        T request;
        T ver;
        std::vector<ApiFramework> api;   ///< Supported APIs (table 5.6)
        std::vector<CreativeAttribute> battr;  ///< Blocked creative attributes (table 5.3)
        T ext; //jsonv::value ext;
    };

    template<typename T>
    struct Metric {
      T type;                                            ///< Type of metric being presented using exchange curated string
                                                         ///< names which should be published to bidders a priori.
      boost::optional<float> value;                      ///< Number representing the value of the metric. Probabilities
                                                         ///< must be in the range 0.0 – 1.0.
      T vendor;
      T ext;                                             ///< Placeholder for exchange-specific extensions to OpenRTB.
    };

    template<typename T>
    struct Impression {
        ~Impression() {}
        T id;                                            ///< Impression ID within BR
        boost::optional<Metric<T>> metric;               ///< An array of Metric object
        boost::optional<Banner<T>> banner;               ///< If it's a banner ad
        boost::optional<Video<T>> video;                 ///< If it's a video ad
        boost::optional<Native<T>> native;               ///< If it's a native ad
        boost::optional<PMP> pmp;                        ///< Containing any Deals eligible for the impression object
        vanilla::unicode_string displaymanager;          ///< What renders the ad
        vanilla::unicode_string displaymanagerver;       ///< What version of that thing
        bool instl{};                                    ///< Is it interstitial
        vanilla::unicode_string tagid;                   ///< ad tag ID for auction //TODO : utf8
        double bidfloor{};                               ///< CPM bid floor
        T bidfloorcur;                                   ///< Bid floor currency
        boost::optional<int> clickbrowser;               ///<Indicates the type of browser opened upon clicking the
                                                         ///< creative in an app, where 0 = embedded, 1 = native

        boost::optional<int> secure;                     ///< Flag that requires secure https assets (1 == yes) (OpenRTB 2.2)
        std::vector<T> iframebuster;                     ///< Supported iframe busters (for expandable/video ads)
        boost::optional<int> exp;                        ///< Advisory as to the number of seconds that may elapse
                                                         ///< between the auction and the actual impression.
        T ext;                                           ///< Extended impression attributes
    };

    template<typename T>
    struct Source {
        int fd{}; ///< Entity responsible for the final impression sale decision, where 0 = exchange, 1 = upstream source.
        T tid;    ///< Transaction ID that must be common across all participants in this bid request
        T pchain; ///< Payment ID chain string containing embedded syntax described in the TAG Payment ID Protocol v1.0.
        T ext;                                           ///< Extended impression attributes
    };

    template<typename T>
    struct BidRequest {
        using request_type = BidRequest<T>;

        ~BidRequest() {}
        T id;                                          ///< Bid request ID
        std::vector<Impression<T>> imp;                ///< List of impressions
        boost::optional<Site<T>> site;
        boost::optional<App<T>> app;
        boost::optional<Device<T>> device;
        boost::optional<User<T>> user;
        int test{};                                     ///< Indicator of test mode in which auctions are not billable,
                                                        ///< where 0 = live mode, 1 = test mode.
        AuctionType at{AuctionType::SECOND_PRICE};      ///< Auction type (1=first/2=second party)
        int tmax{};                                     ///< Max time avail in ms
        std::vector<T> wseat;                           ///< Allowed buyer seats
        std::vector<T> bseat;                           ///< Block list of buyer seats
        bool allimps{};                                 ///< All impressions in BR (for road-blocking)
        std::vector<T> cur;                             ///< Allowable currencies
        std::vector<T> wlang;                           ///< White list of languages for creatives using ISO-639-1-alpha-2.
        std::vector<ContentCategory<T>> bcat;           ///< Blocked advertiser categories (table 6.1)
        std::vector<T> badv;                            ///< Blocked advertiser domains
        std::vector<T> bapp;                            ///< Block list of applications by their platform-specific exchangeindependent application identifiers
        boost::optional<Source<T>> source;              ///< A Sorce object
        boost::optional<Regulations<T>> regs;           ///< Regulations Object list (OpenRTB 2.2)
        T ext; //jsonv::value ext;                      ///< Protocol extensions
        T unparseable; //jsonv::value unparseable;      ///< Unparseable fields get put here
        
        const request_type& request() const {
            return *this;
        }
    };


//OpenRTB 2.2 Response structures

    template<typename T>
    struct Bid {
        T id;                               ///< Bidder's bid ID to identify bid
        T impid;                            ///< ID of the impression we're bidding on
        double price{};                     ///< Price to bid
        T nurl;                             ///< Win notice/ad markup URL
        T burl;                             ///< Billing notice URL
        T lurl;                             ///< Loss notice URL
        T adm;                              ///< Ad markup
        T adid;                             ///< Id of ad to be served if won
        std::vector<T> adomain;             ///< Advertiser domains
        T bundle;                           ///< A platform-specific application identifier
        T iurl;                             ///< Image URL for content checking
        T cid;                              ///< Campaign ID
        T crid;                             ///< Creative ID
        T tactic;                           ///< Tactic ID to enable buyers to label bids for reporting to the
                                            ///< exchange the tactic through which their bid was submitted.
        std::vector<ContentCategory<T>> cat;///< Content categories
        std::vector<CreativeAttribute> attr;///< Creative attributes
        int api{};                          ///< API required by the markup if applicable.
        int protocol{};                     ///< Video response protocol of the markup if applicable
        int qagmediarating{};               ///< Creative media rating per IQG guidelines.
        T language;                         ///< Language of the creative
        T dealid;                           ///< unique id for the deal associated with bid
                                            ///< if its in bid request, required in bid response
        int w{};                            ///< Width of ad
        int h{};                            ///< Height of ad
        int wratio{};                       ///< Relative width of the creative when expressing size as a ratio.
                                            ///< Required for Flex Ads.
        int hratio{};                       ///< Relative height of the creative when expressing size as a ratio.
                                            ///< Required for Flex Ads.
        int exp{};                          ///< Advisory as to the number of seconds the bidder is willing to
                                            ///< wait between the auction and the actual impression.
        T ext; //jsonv::value ext;          ///< Extended bid fields
    };

    template<typename T>
    struct SeatBid {
        std::vector<Bid<T>> bid;  ///< Array of bid objects  (relating to imps)
        T seat;      ///< Seat on behalf of whom the bid is made
        int group{};            ///< If true, imps must be won as a group
        T ext; //jsonv::value ext;     ///< Extension fields
    };

    template<typename T>
    struct BidResponse {
        using data_type = T;
        T id;
        std::vector<SeatBid<T>> seatbid;
        T bidid;
        T cur;
        T customdata;
        NoBidReason nbr{NoBidReason::UNKNOWN_ERROR}; ///< reason for not bidding
        T ext; //jsonv::value ext; //Placeholder for bidder-specific extensions to OpenRTB

        void clear() { *this = {}; }
    };

}
