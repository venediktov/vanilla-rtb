
#include "exchange/exchange_handler.hpp"
#include "DSL/generic_dsl.hpp"

struct A{};

int main() {
    vanilla::exchange::connection_endpoint ep {std::make_tuple("0.0.0.0" , "8081" , ".")} ;
    vanilla::exchange::exchange_handler<A> ehandler(ep, boost::regex("/venue_handler/(\\w+)/(\\d+)" ) );
    ehandler.run() ;
}


