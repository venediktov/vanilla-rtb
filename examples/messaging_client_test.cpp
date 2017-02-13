
#include <iostream>
#include <chrono>
#include <iterator>
#include <iterator>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include "core/openrtb.hpp"
#include "messaging/serialization.hpp"
#include "messaging/communicator.hpp"
#include "DSL/generic_dsl.hpp"

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

namespace po = boost::program_options;
using namespace vanilla::messaging;
using namespace std::literals;

int main(int argc, char**argv) {

  init_framework_logging("/tmp/openrtb_messaging_test_log");

  std::string remote_address;
  short port;
  po::options_description desc;
        desc.add_options()
            ("help", "produce help message")
            ("remote_address",po::value<std::string>(&remote_address), "respond to remote address")
            ("port", po::value<short>(&port), "port")
        ;

  boost::program_options::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch ( const std::exception& ) {
    LOG(error) << desc;
    return 1;
  }

  if (vm.count("help")) {
      LOG(info) << desc;
      return 0;
  }


//This code below can be placed in  exchange_handler_test.cpp inside auction handler
std::vector<std::string> responses;

communicator<broadcast>()
.outbound(port)
.distribute(openrtb::BidRequest())
.collect(10ms, [&responses](const std::string serialized_data) {
    std::stringstream ss (serialized_data);
    boost::archive::binary_iarchive iarch(ss);
    openrtb::BidResponse bid;
    iarch >> bid;
    auto resp =  to_string(DSL::GenericDSL().create_response(bid)) ;
    LOG(info) << "Received back:" << resp;
    responses.emplace_back(resp.data(),resp.size());
});

std::copy ( std::begin(responses), std::end(responses), std::ostream_iterator<std::string>(std::cout, "\n"));

}

