# vanilla-rtb

Real Time Bidding (RTB) - Demand Side Platform framework

open-source library utilizing  modern C++11/14  features and latest Boost.

What makes us different from other open-source RTB projects we have seen?

Our stack is fairly small and 
easy to integrate with [your cmake project](https://github.com/vanilla-rtb/rapid-bidder) 
, completely decoupled by use of templates and has minimum dependency. 


[vanilla-rtb ecosystem](../../wiki)

[Multi-bidder-model-with-communicator-for-Win-notifications](../../wiki/Multi-bidder-model-with-communicator-for-Win-notifications)

[![Join the chat at https://gitter.im/vanilla-rtb/Lobby](https://badges.gitter.im/vanilla-rtb/Lobby.svg)](https://gitter.im/vanilla-rtb/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) 
[![build ](https://travis-ci.org/venediktov/vanilla-rtb.svg?branch=master)](https://travis-ci.org/venediktov/vanilla-rtb)

Structure ( work in progress ) :
* [/](../../tree/master/) -- the root directory
   * [boost-process/](../../tree/master/boost-process/) -- C++11 planned for official boost stack version 1.64
   * [boost-dll/](../../tree/master/boost-dll/) -- C++11 in official boost stack since version 1.61
   * [CRUD/](../../tree/master/CRUD/) -- Restful web-service written in C++11 based on boost.ASIO and CRUD handlers
   * [rtb/](../../tree/master/rtb/) -- C++11 framework and sandbox for testing platform solution 
      * [core/](../../tree/master/rtb/core/) -- generic structures shared in the project ( RTB specific )
      * [common/](../../tree/master/rtb/common) -- generic RTB agnostic structures 
      * [datacache/](../../tree/master/rtb/datacache/) -- IPC data store for fast lookups and matching
      * [exchange/](../../tree/master/rtb/exchange) -- exchange handlers implementation
      * [DSL/](../../tree/master/rtb/DSL) --  DSL formats for jsonv
    * [examples/](../../tree/master/examples) -- root to our sandbox with examples
      * [bidder/](../../tree/master/examples/bidder) -- collection of application specific classes to support targeting
      * [loader/](../../tree/master/examples/loader) -- collection of application specific classes to support campaign loading
      * [campaign/](../../tree/master/examples/campaign) -- add/modify/delete campaign API + UI ( work in progress ) 
      
* [CMakeLists.txt] - cmake file

>The stack of vanilla-rtb depends on other C++11 projects and is referencing them via gh-subree.
>To update to the latest version of boost-process , boost-dll , json-voorhees or CRUD  use the following commands \:

* git subtree pull --prefix jsonv git@github.com:tgockel/json-voorhees.git master --squash
* git subtree pull --prefix boost-process git@github.com:BorisSchaeling/boost-process.git master --squash
* git subtree pull --prefix boost-dll git@github.com:apolukhin/Boost.DLL.git master --squash
* git subtree pull --prefix CRUD git@github.com:venediktov/CRUD.git  master --squash



### *(&#x1F4D7;) To build vanilla-rtb use following commands in the root of vanilla-rtb*

### Linux \:
- [x] mkdir Release
- [x] cd Release
- [x] cmake -DCMAKE_BUILD_TYPE=Release .. -G "Unix Makefiles"
- [x] gmake VERBOSE=1
- [x] cd ..
- [x] mkdir Debug
- [x] cd Debug
- [x] cmake -DCMAKE_BUILD_TYPE=Debug .. -G "Unix Makefiles"
- [x] gmake VERBOSE=1

### Windows \:
*same steps as above for linux , only difference is depending on your environment 
  either Visual Studio or NMake project can be used*
  
- [x] cmake -DCMAKE_BUILD_TYPE=Release .. -G "NMake Makefiles"
- [x] cmake -DCMAKE_BUILD_TYPE=Debug   .. -G "NMake Makefiles"
- [x] cmake -DCMAKE_BUILD_TYPE=Release .. -G "Visual Studio 14 2015"
- [x] cmake -DCMAKE_BUILD_TYPE=Debug   .. -G "Visual Studio 14 2015"


### For faster builds invoking multiple make processes  , find number of cores on your system
Linux command \: 
* nproc

4

pass it to your make script like this

**gmake -j4 install**

### Running examples\:
- [x] HTTP-Bidder
  * vanilla-rtb/Release/examples/bin$ ./http_bidder_test --config etc/config.cfg
- [x] Cache loader
  * vanilla-rtb/Release/examples/bin$ ./cache_loader_test --config etc/config.cfg
- [x] Exchange Handler with HTTP handler or Exchange Handler distributing to multi-bidders via communicator 
  * vanilla-rtb/Release/examples/bin$ ./exchange_handler_test --config etc/config.cfg
- [x] Mock-bidders starting multiple in one swoop,  currently configured as 5 bidders in config
  * vanilla-rtb/Release/examples/bin$ ./mock_bidder_test --config etc/config.cfg
- [x] multi-bidders starting multiple in one swoop,  currently configured as 3 bidders in config and exchange
  * vanilla-rtb/Release/examples/bin$ ./multi_bidder --config etc/config.cfg
  * vanilla-rtb/Release/examples/bin$ ./multi_exchange_handler --config etc/config.cfg
- [x] Notification service and Slave-Banker
  * vanilla-rtb/Release/examples/bin$ ./notification_service_test --config etc/config.cfg
  * vanilla-rtb/Release/examples/bin$ ./slavebanker_service_test --config etc/config.cfg

