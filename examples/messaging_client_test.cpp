
#include <iostream>
#include <chrono>
#include <iterator>
#include <iterator>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include "core/openrtb.hpp"
#include "common/perf_timer.hpp"
#include "messaging/serialization.hpp"
#include "messaging/communicator.hpp"
#include "DSL/generic_dsl.hpp"

#define LOG(x) BOOST_LOG_TRIVIAL(x) //TODO: move to core.hpp

extern void init_framework_logging(const std::string &) ;

namespace po = boost::program_options;
using namespace vanilla::messaging;
using namespace std::literals;

namespace openrtb {
    template<typename T> class BidResponse;

    template<typename T>
    std::ostream& operator<< (std::ostream &os, const BidResponse<T> &bid) {
        os << to_string(DSL::GenericDSL<T>().create_response(bid));
        return os;
    }
}

int main(int argc, char**argv) {

  using BidRequest = openrtb::BidRequest<std::string>;
  using BidResponse = openrtb::BidResponse<std::string>;
  init_framework_logging("/tmp/openrtb_messaging_test_log");

  std::string remote_address{};
  short port{};
  unsigned int n_bid{};
  po::options_description desc;
        desc.add_options()
            ("help", "produce help message")
            ("remote_address",po::value<std::string>(&remote_address), "respond to remote address")
            ("port", po::value<short>(&port), "port")
            ("n", po::value<unsigned int>(&n_bid)->required(), "number of bidders to wait-for")
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
std::vector<BidResponse> responses;
auto sp = std::make_shared<std::stringstream>();
{
    perf_timer<std::stringstream> timer(sp);
    communicator<broadcast>()
    .outbound(port)
    .distribute(BidRequest())
    .collect<BidResponse>(10ms, [&responses,n_bid](BidResponse bid, auto done) {
        responses.push_back(bid);
        if (responses.size() == n_bid) {
            done();
        }
    });
}
LOG(debug) << sp->str();
std::copy ( std::begin(responses), std::end(responses), std::ostream_iterator<BidResponse>(std::cout, "\n"));

}

