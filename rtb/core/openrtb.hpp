#pragma once

#include "core/unicode_string.hpp"
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
    struct Banner {
        ~Banner() {}

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
        std::vector<MimeType<T>> mimes;                    ///< Whitelist of content MIME types
        uint32_t minduration{};                         /// Minimum video ad duration in seconds.
        uint32_t maxduration{};                         /// Maximum video ad duration in seconds.
        std::vector<Protocol> protocols;           /// Array of supported video protocols. 
        Protocol protocol;                         /// Deprecated  in favor of protocols
        uint16_t w{};                                   /// Width of the video player in device independent pixels (DIPS).
        uint16_t h{};                                   /// Height of the video player in device independent pixels (DIPS).
        uint32_t startdelay{};                          /// Indicates the start delay in seconds for pre-roll, mid-roll, or
                                                        /// post-roll ad placements. 
        VideoPlacement placement;                       /// Placement type for the impression. 
        VideoLinearity linearity;                       /// Indicates if the impression must be linear, nonlinear, etc. If
                                                        /// none specified, assume all are allowed. 
        uint8_t skip{0};                                /// Indicates if the player will allow the video to be skipped,
        uint32_t skipmin{};                             /// Videos of total duration greater than this number of seconds
                                                        /// can be skippable; only applicable if the ad is skippable.
        uint32_t skipafter{};                           /// Number of seconds a video must play before skipping is
                                                        /// enabled; only applicable if the ad is skippable.
        uint16_t sequence;                              /// If multiple ad impressions are offered in the same bid request,
                                                        /// the sequence number will allow for the coordinated delivery
                                                        /// of multiple creatives.
        std::vector<CreativeAttribute> battr;           /// Blocked creative attributes.

        int maxextended{};                              /// Maximum extended ad duration if extension is allowed. If
                                                        ///blank or 0, extension is not allowed. If -1, extension is
                                                        /// allowed, and there is no time limit imposed. If greater than 0,
                                                        /// then the value represents the number of seconds of extended
                                                        /// play supported beyond the maxduration value.
        uint32_t minbitrate{};                          /// Minimum bit rate in Kbps.
        uint32_t maxbitrate{};                          /// Maximum bit rate in Kbps.
        uint8_t boxingallowed{1};                       /// Indicates if letter-boxing of 4:3 content into a 16:9 window is
                                                        /// allowed, where 0 = no, 1 = yes.
        std::vector<PlaybackMethod> playbackmethod;     /// Playback methods that may be in use. If none are specified,
                                                        /// any method may be used. Refer to List 5.10. Only one
                                                        /// method is typically used in practice. As a result, this array may
                                                        /// be converted to an integer in a future version of the
                                                        /// specification. It is strongly advised to use only the first
                                                        /// element of this array in preparation for this change.
        PlaybackCessationModes playbackend;             /// The event that causes playback to end.
        DeliveryMethod delivery;                         /// Supported delivery methods (e.g., streaming, progressive). If
                                                        /// none specified, assume all are supported. 
        AdPosition pos;                                 /// Ad position on screen. 
        CompanionType companiontype;                    /// Supported VAST companion ad types.
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
    struct Context {
        //T id;                 ///< Site ID on the exchange
        T name; //vanilla::unicode_string name;  ///< Site name
        T domain; //vanilla::unicode_string domain;///< Site or app domain
        std::vector<ContentCategory<T>> cat;        ///< IAB content categories for site/app
        std::vector<ContentCategory<T>> sectioncat; ///< IAB content categories for subsection
        std::vector<ContentCategory<T>> pagecat;    ///< IAB content categories for page/view
        bool privacypolicy;           ///< Has a privacy policy
        boost::optional<Publisher<T>> publisher;    ///< Publisher of the site or app
                                                 //boost::optional<Content> content;        ///< Content of the site or app
        std::vector<T> keywords;                    ///< Keywords describing app
        T ext; //jsonv::value ext;
    };

    template<typename T>
    struct SiteImpl {
        T page;          ///< URL of the page to be shown
        T ref;           ///< Referrer URL that got user to page
        //vanilla::unicode_string search; ///< Search string that got user to page
    };

    template<typename T>
    struct Site : Context<T>, SiteImpl<T>   {
        T id;
    };
    enum class GeoType : int8_t {
        UNDEFINED = -1,  ///< Not explicitly specified

        GPS = 1,        ///< GPS/Location Services
        IP = 2,         ///< IP Address
        USER = 3        ///< User provided (e.g., registration data)
    };
    template<typename T>
    struct Geo {
        float lat{};                        ///< Latitude from -90.0 to +90.0, where negative is south.
        float lon{};                        ///< Longitude from -180.0 to +180.0, where negative is west.
        GeoType type{GeoType::UNDEFINED};   ///< Source of location data; recommended when passing lat/lon
        int utcoffset{};                    ///< Local time as the number +/- of minutes from UTC.
        T city;                   ///< City using United Nations Code for Trade & Transport
                                            ///  Locations. See Appendix A for a link to the codes.
        T country;                ///< Country code using ISO-3166-1-alpha-3
        T region;                 ///< Region code using ISO-3166-2; 2-letter state code if USA.
        T regionfips104;          ///< Region of a country using FIPS 10-4 notation. While OpenRTB
                                            ///  supports this attribute, it has been withdrawn by NIST in 2008.
        T metro;                  ///< Google metro code; similar to but not exactly Nielsen DMAs.
                                            ///  See Appendix A for a link to the codes
        T zip;                    ///< Zip or postal code
        T ext; //jsonv::value ext;                   ///< Placeholder for exchange-specific extensions to OpenRTB.
        
        
    };
    struct App {};
    struct Device {};
    
    template<typename T>
    struct UserDataSegment {
        T id;                         ///< ID of the data segment specific to the data provider.
        T name;                       ///< Name of the data segment specific to the data provider.
        T value;                      ///< String representation of the data segment value.
        T ext; //jsonv::value ext;                       ///< Placeholder for exchange-specific extensions to OpenRTB.
    };
    template<typename T>
    struct UserData {
        T id;                         ///< Exchange-specific ID for the data provider
        T name;                       ///< Exchange-specific name for the data provider
        std::vector<UserDataSegment<T>> segment;   ///< Array of Segment objects that contain the actual data values.
        T ext; //jsonv::value ext;           ///< Placeholder for exchange-specific extensions to OpenRTB.
    };
    template<typename T>
    struct User {
        int yob{};                  ///< Year of birth as a 4-digit integer
        T id;             ///< Exchange-specific ID for the user.
        T buyeruid;       ///< Buyer-specific ID for the user as mapped by the exchange for the buyer
        T gender;         ///< Gender, where “M” = male, “F” = female, “O” = known to be other (i.e., omitted is unknown).
        T keywords;       ///< Comma separated list of keywords, interests, or intent
        T customdata;     ///< Optional feature to pass bidder data that was set in the
                                    ///  exchange’s cookie. The string must be in base85 cookie safe
                                    ///  characters and be in any format. Proper JSON encoding must
                                    ///  be used to include “escaped” quotation marks.
        boost::optional<Geo<T>> geo;
        std::vector<UserData<T>> data; ///< Additional user data
        T ext; //jsonv::value ext;           ///< Placeholder for exchange-specific extensions to OpenRTB.
        
    };

    struct Regulations {};
    
    template<typename T>
    struct Native {
        T request;
        T ver;
        std::vector<ApiFramework> api;   ///< Supported APIs (table 5.6)
        std::vector<CreativeAttribute> battr;  ///< Blocked creative attributes (table 5.3)
        T ext; //jsonv::value ext;
    };

    template<typename T>
    struct Impression {
        ~Impression() {}
        T id;                             ///< Impression ID within BR
        boost::optional<Banner<T>> banner;           ///< If it's a banner ad
        boost::optional<Video<T>> video;             ///< If it's a video ad
        boost::optional<Native<T>> native;           ///< If it's a native ad
        vanilla::unicode_string displaymanager;          ///< What renders the ad
        vanilla::unicode_string displaymanagerver;        ///< What version of that thing
        bool instl{};            ///< Is it interstitial
        vanilla::unicode_string tagid;                   ///< ad tag ID for auction //TODO : utf8
        double bidfloor{};        ///< CPM bid floor
        T bidfloorcur;                ///< Bid floor currency
        int  secure{};           ///< Flag that requires secure https assets (1 == yes) (OpenRTB 2.2)
        std::vector<T> iframebuster;         ///< Supported iframe busters (for expandable/video ads)
        boost::optional<PMP> pmp;        ///< Containing any Deals eligible for the impression object
        T ext; //jsonv::value ext;                   ///< Extended impression attributes
    };

    template<typename T>
    struct BidRequest {
        using request_type = BidRequest<T>;
        
        ~BidRequest() {}
        T id;                             ///< Bid request ID
        std::vector<Impression<T>> imp;            ///< List of impressions
        boost::optional<Site<T>> site;
        boost::optional<App> app;
        boost::optional<Device> device;
        boost::optional<User<T>> user;
        AuctionType at;                    ///< Auction type (1=first/2=second party)
        int tmax{};                    ///< Max time avail in ms
        std::vector<T> wseat;              ///< Allowed buyer seats
        bool allimps{};                ///< All impressions in BR (for road-blocking)
        std::vector<T> cur;                ///< Allowable currencies
        std::vector<ContentCategory<T>> bcat;        ///< Blocked advertiser categories (table 6.1)
        std::vector<vanilla::unicode_string> badv;           ///< Blocked advertiser domains
        boost::optional<Regulations> regs; ///< Regulations Object list (OpenRTB 2.2)
        T ext; //jsonv::value ext;                   ///< Protocol extensions
        T unparseable; //jsonv::value unparseable;           ///< Unparseable fields get put here
        
        const request_type& request() const {
            return *this;
        }
    };


//OpenRTB 2.2 Response structures

    template<typename T>
    struct Bid {
        T id;                       ///< Bidder's bid ID to identify bid
        T impid;                    ///< ID of the impression we're bidding on
        double price{};                       ///< Price to bid
        T adid;                     ///< Id of ad to be served if won
        T nurl;                     //vanilla::unicode_string nurl;                  ///< Win notice/ad markup URL
        T adm;                      //vanilla::unicode_string adm;                   ///< Ad markup
        std::vector<T> adomain;     ///< Advertiser domains
        T iurl;                     //vanilla::unicode_string iurl;                  ///< Image URL for content checking
        T cid;                      ///< Campaign ID
        T crid;                     ///< Creative ID
        std::vector<CreativeAttribute> attr;  ///< Creative attributes
        T dealid;                   ///< unique id for the deal associated with bid
                                              ///< if its in bid request, required in bid response
        int w{};                              ///< Width of ad
        int h{};                              ///< Height of ad
        T ext; //jsonv::value ext;                     ///< Extended bid fields
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
        NoBidReason nbr; ///< reason for not bidding
        T ext; //jsonv::value ext; //Placeholder for bidder-specific extensions to OpenRTB
        
        void clear() {
            id.clear();
            seatbid.clear();
            bidid.clear();
            cur.clear();
            customdata.clear();
            nbr = NoBidReason::UNKNOWN_ERROR;
            ext.clear();
        }
    };

}
