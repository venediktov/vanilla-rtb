
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/program_options.hpp>
#include "exchange/exchange_handler.hpp"
#include "exchange/exchange_server.hpp"
#include "CRUD/handlers/crud_dispatcher.hpp"
#include "DSL/generic_dsl.hpp"
#include "rtb/config/config.hpp"
#include "core/tagged_tuple.hpp"
#include "datacache/ad_entity.hpp"
#include "datacache/entity_cache.hpp"
#include "datacache/memory_types.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <vector>

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

struct bidder_config_data {
    std::string log_file_name;
    int timeout;
    
    bidder_config_data() :
        log_file_name{}, timeout{}
    {}
};

struct Ad {
    std::string ad_id;
    uint16_t width;
    uint16_t height;
    uint32_t position;
    uint64_t max_bid_micros;
    std::string record;
    
    Ad(std::string ad_id, uint16_t width, uint16_t height, uint32_t position, uint64_t max_bid_micros) : 
        ad_id{std::move(ad_id)}, width{width}, height(height), position{position}, max_bid_micros{max_bid_micros}, record{}
    {}
    Ad():
        ad_id{}, width{}, height{}, position{}, max_bid_micros{}, record{}
    {}
        
    template<typename T>
    friend std::ostream &operator<<(std::ostream & os, const std::shared_ptr<T> & t)  {
        os <<  *t ;
        return os;
    }
    friend std::ostream &operator<<(std::ostream & os, const  Ad & value)  {
        os << value.ad_id << "|" 
           << value.width << "|" 
           << value.height << "|" 
           << value.position << "|" 
           << value.max_bid_micros << "|" 
        ;
        return os;
    }
    friend std::istream &operator>>(std::istream &is, Ad &l) {
        if ( !std::getline(is, l.record) ){
            return is;
        }
        std::vector<std::string> fields;
        boost::split(fields, l.record, boost::is_any_of(","), boost::token_compress_on);
        l.ad_id = fields.at(0); 
        l.width = atoi(fields.at(1).c_str());
        l.height = atoi(fields.at(2).c_str());
        l.position = atol(fields.at(3).c_str());
        l.max_bid_micros = atol(fields.at(4).c_str());
        return is;
    }
};

template<typename Keys, typename Cache>
void populate_cache(std::vector<Ad> ads , Cache && cache) {
    for ( auto &value : ads ) {
       if ( cache.insert(Keys{value.ad_id}, value) ) {
         LOG(info) << "inserted{" << value.ad_id  << "} OK!" ; 
       } else {
          LOG(error) << "insert {" << value.ad_id << "} FAILED!" ;
       }
    }
}


//Non-Intrusive boost serialization implementation
namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, Ad & value, const unsigned int version)
{
    ar & value.ad_id;
    ar & value.width;
    ar & value.height;
    ar & value.position;
    ar & value.max_bid_micros;
    ar & value.record;
}

} // namespace serialization
} // namespace boost
int main(int argc, char *argv[]) {
    using namespace std::placeholders;
    using namespace vanilla::exchange;
    using namespace std::chrono_literals;
    using restful_dispatcher_t =  http::crud::crud_dispatcher<http::server::request, http::server::reply> ;
    
    using Memory = mpclmi::ipc::Shared;
    using Cache = datacache::entity_cache<Memory, ipc::data::ad_container> ; 
    using Alloc = typename Cache::char_allocator ;
    using Tag = typename ipc::data::ad_entity<Alloc>::ad_id_tag;
    using Keys = vanilla::tagged_tuple
    <
        Tag, std::string
    >;
    
    vanilla::config::config<bidder_config_data> config([](bidder_config_data &d, boost::program_options::options_description &desc){
        desc.add_options()
            ("bidder.log", boost::program_options::value<std::string>(&d.log_file_name), "bidder_test log file name log")
            ("bidder.host", "bidder_test Host")
            ("bidder.port", "bidder_est Port")
            ("bidder.root", "bidder_test Root")
            ("bidder.timeout", boost::program_options::value<int>(&d.timeout), "bidder_test timeout")
        ;
    });
    
    try {
        config.parse(argc, argv);
    }
    catch(std::exception const& e) {
        LOG(error) << e.what();
        return 0;
    }
    LOG(debug) << config;
    init_framework_logging(config.data().log_file_name);
    
    exchange_handler<DSL::GenericDSL> bid_handler(std::chrono::milliseconds(config.data().timeout));
    bid_handler    
        .logger([](const std::string &data) {
            //LOG(debug) << "bid request=" << data ;
        })
        .error_logger([](const std::string &data) {
            LOG(debug) << "bid request error " << data ;
        })
        .auction([](const openrtb::BidRequest &request) {
            for(auto &imp : request.imp) {
                LOG(debug) << "Request floor " << imp.bidfloor;
                if(imp.banner) {
                    LOG(debug) << "ad " << imp.banner.get().w << "x" << imp.banner.get().h;
                }
            }
            
            openrtb::BidResponse response;
            return response;
        });
    
    connection_endpoint ep {std::make_tuple(config.get("bidder.host"), config.get("bidder.port"), config.get("bidder.root"))};

    //initialize and setup CRUD dispatchers
    restful_dispatcher_t dispatcher(ep.root) ;
    dispatcher.crud_match(boost::regex("/bid/(\\d+)"))
              .post([&](http::server::reply & r, const http::crud::crud_match<boost::cmatch> & match) {
                  bid_handler.handle_post(r,match);
              });

    exchange_server<restful_dispatcher_t> server{ep,dispatcher} ;
    server.run() ;
}


