
#include <unistd.h>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include "rtb/core/openrtb.hpp"
#include "rtb/messaging/communicator.hpp"
#include "rtb/messaging/serialization.hpp"
#include "rtb/config/config.hpp"

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

namespace po = boost::program_options;
using namespace vanilla::messaging;

int main(int argc, char**argv) {

  init_framework_logging("/tmp/openrtb_mock_bidder_test_log");

  std::string local_address;
  std::string group_address;
  std::string type;
  unsigned short port;

  vanilla::config::config<void *> config([&](void *&d, boost::program_options::options_description &desc){
    desc.add_options()
        ("help", "produce help message")
        ("mock-bidder.local_address", po::value<std::string>(&local_address)->default_value("0.0.0.0"), "bind to this address locally")
        ("mock-bidder.group_address",po::value<std::string>(&group_address)->default_value("0.0.0.0"), "join on remote address (only used for multicast)")
        ("mock-bidder.port", po::value<unsigned short>(&port)->required(), "port")
        ("mock-bidder.communicator.type", po::value<std::string>(&type)->default_value("broadcast"), "communication types : multicast , broadcast")
    ;
  });

  try {
     config.parse(argc, argv);
  } catch(std::exception const& e) {
     LOG(error) << e.what();
     return 1;
  }
  LOG(debug) << config;


  LOG(info) << "Starting mock bidder pid=" << getpid();

  //std::string data is serialized and moved constructed
  communicator<broadcast>().inbound(port).process([](auto endpoint, std::string data) { 
      LOG(info) << "Received(Broadcast:" << *endpoint  << "):" << data;
      return openrtb::BidResponse();
  }).dispatch();

}

