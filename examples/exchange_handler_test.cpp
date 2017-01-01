
#include "exchange/exchange_handler.hpp"
#include "DSL/generic_dsl.hpp"

int main() {
    std::string host("0.0.0.0") ;
    std::string port("8081")  ;
    std::string root(".") ;
    boost::regex rgx("/openrtb_handler/(\\w+)/(\\d+)");
    vanilla::exchange::connection_endpoint ep {std::make_tuple(host, port, root)} ;
    vanilla::exchange::exchange_handler<DSL::GenericDSL> ehandler(ep, rgx);
    ehandler.run() ;
}


