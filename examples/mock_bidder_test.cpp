

#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include "rtb/core/openrtb.hpp"
#if !defined(WIN32)
#include <unistd.h>
#include "rtb/core/process.hpp"
#else
#include <process.h>
#endif
#include "rtb/messaging/communicator.hpp"
#include "rtb/messaging/serialization.hpp"
#include "rtb/config/config.hpp"

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

namespace po = boost::program_options;
using namespace vanilla::messaging;

void run_communicator(unsigned short port) ;

int main(int argc, char**argv) {

  init_framework_logging("/tmp/openrtb_mock_bidder_test_log");

  std::string local_address;
  std::string group_address;
  std::string type;
  unsigned short port{};
  int num_of_bidders{};

  vanilla::config::config<void *> config([&](void *&d, boost::program_options::options_description &desc){
    desc.add_options()
        ("help", "produce help message")
        ("mock-bidder.local_address", po::value<std::string>(&local_address)->default_value("0.0.0.0"), "bind to this address locally")
        ("mock-bidder.group_address",po::value<std::string>(&group_address)->default_value("0.0.0.0"), "join on remote address (only used for multicast)")
        ("mock-bidder.port", po::value<unsigned short>(&port)->required(), "port")
        ("mock-bidder.communicator.type", po::value<std::string>(&type)->default_value("broadcast"), "communication types : multicast , broadcast")
        ("mock-bidder.num_of_bidders", po::value<int>(&num_of_bidders)->default_value(1), "number of bidders to spawn")
    ;
  });

  try {
     config.parse(argc, argv);
  } catch(std::exception const& e) {
     LOG(error) << e.what();
     return 1;
  }
  LOG(debug) << config;
#if !defined(WIN32)
  using OS::UNIX::Process;
  try {    
      auto handle = []( unsigned int port ) { run_communicator(port) ; } ;
      using Handler = decltype(handle) ;
      Process<> parent_proc;
      Process<Handler> child_proc(handle) ;
      int thresh_hold  = config.get<int>("mock-bidder.num_of_bidders") ;
      std::vector<decltype(child_proc)> child_procs(thresh_hold, child_proc) ;
      auto spawned_procs = parent_proc.spawn(child_procs, port) ;
      parent_proc.wait(spawned_procs) ;
  } catch(const std::exception &e) {
      LOG(error) << e.what(); 
      return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
#else
  run_communicator(port) ; //TODO: need suport for spawning procs in Windows
#endif
}

void run_communicator(unsigned short port) {
  using BidRequest  = openrtb::BidRequest<jsonv::string_view>;
  using BidResponse = openrtb::BidResponse<jsonv::string_view>;
  LOG(info) << "Starting mock bidder pid=" << getpid();
  communicator<broadcast>().inbound(port).process<BidRequest>([](auto endpoint, BidRequest data) {
      //LOG(info) << "Received(Broadcast:" << *endpoint  << "):" << data ;
      return BidResponse();
  }).dispatch();
}

