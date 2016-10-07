namespace DSL {
    
    class GenericDSL {
    public:
    GenericDSL() {
      using namespace openrtb;
      formats base = formats_builder()
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
          .type<Site>()
          .member("id", &Site::id)
          .type<BidRequest>()
          .member("id", &BidRequest::id)
          .member("imp", &BidRequest::imp)
          .member("site", &BidRequest::site)
          .encode_if([](const jsonv::serialization_context&, const boost::optional<Site>& x) {return bool(x);})
          .register_container<std::vector<Impression>>()
          .register_container<boost::optional<Banner>>()
          .register_container<boost::optional<Site>>()
          .register_container<boost::optional<Publisher>>()
          .register_container<std::vector<std::string>>()
          .register_container<std::vector<int>>()
          .check_references(formats::defaults())
          ;

              fmt_ = formats::compose({ base, formats::defaults() });
          }

       openrtb::BidRequest extract_request (const std::string & bid_request) {
           auto encoded = parse(bid_request);
           return extract<openrtb::BidRequest>(encoded, fmt_);
       }

      private :
          formats fmt_;
};
