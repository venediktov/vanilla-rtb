# vanilla-rtb

Real Time Bidding (RTB) - Demand Side Platform framework

open-source library utilizing  modern C++11/14  features and latest Boost.

What makes us different from other open-source RTB projects we have seen?

Our stack is fairly small and easy to integrate with any cmake project. 
As a model project please see [https://github.com/vanilla-rtb/rapid-bidder](https://github.com/vanilla-rtb/rapid-bidder)

vanilla-rtb stack is completely decoupled by C++ templates and has minimum dependency on outside vendors.

[vanilla-rtb ecosystem](../../wiki)

[Multi-bidder-model-with-communicator-for-Win-notifications](../../wiki/Multi-bidder-model-with-communicator-for-Win-notifications)

[relatively high - 50K QPS](../../wiki/QPS-test)

[![Join the chat at https://gitter.im/vanilla-rtb/Lobby](https://badges.gitter.im/vanilla-rtb/Lobby.svg)](https://gitter.im/vanilla-rtb/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) 
[![build ](https://travis-ci.org/venediktov/vanilla-rtb.svg?branch=master)](https://travis-ci.org/venediktov/vanilla-rtb)

Recommended build environment: Linux or macOS, CMake - 3.7.2, GCC - 5.1, Boost - 1.60.

Structure :
* [/](../../tree/master/) -- the root directory
   * [boost-process/](../../tree/master/boost-process/) -- C++11 planned for official boost release version 1.64
   * [boost-dll/](../../tree/master/boost-dll/) -- C++11 in official boost release since version 1.61
   * [CRUD/](../../tree/master/CRUD/) -- C++11 high performance HTTP-restful handlers based on boost.ASIO and CRUD API
   * [jsonv/](../../tree/master/jsonv/) -- DSL mapper of json encoded objects to C++ structures
   * [parsers/](../../tree/master/parsers/) -- fast zero copy, zero memory allocation parsers
   * [rtb/](../../tree/master/rtb/) -- RTB framework
      * [core/](../../tree/master/rtb/core/) -- generic structures shared in the project ( RTB specific )
      * [common/](../../tree/master/rtb/common) -- generic RTB agnostic structures 
      * [datacache/](../../tree/master/rtb/datacache/) -- IPC data store for fast targeting lookups
      * [exchange/](../../tree/master/rtb/exchange) -- exchange handlers implementation
      * [DSL/](../../tree/master/rtb/DSL) --  DSL formats for jsonv
   * [docker/](../../tree/master/docker/) -- vanilla docker files and instructions on how to build and run 
   * [examples/](../../tree/master/examples) -- root to our sandbox with examples
      * [bidder/](../../tree/master/examples/bidder) -- collection of application specific classes to support targeting
      * [loader/](../../tree/master/examples/loader) -- collection of application specific classes to support campaign loading
      * [campaign/](../../tree/master/examples/campaign) -- add/modify/delete campaign API + UI ( work in progress ) 
      
* [CMakeLists.txt] - cmake file

>The stack of vanilla-rtb includes other C++11 projects and is referencing them via gh-subree.
>To update to the latest version of boost-process , boost-dll , json-voorhees or CRUD we use the following commands \:

* git subtree pull --prefix jsonv git@github.com:tgockel/json-voorhees.git master --squash
* git subtree pull --prefix boost-process git@github.com:BorisSchaeling/boost-process.git master --squash
* git subtree pull --prefix boost-dll git@github.com:apolukhin/Boost.DLL.git master --squash
* git subtree pull --prefix CRUD git@github.com:venediktov/CRUD.git  master --squash



### *(&#x1F4D7;) To build vanilla-rtb use following commands in the root of vanilla-rtb*

### Linux \:

```bash
$ mkdir Release
$ cd Release
$ cmake -DCMAKE_BUILD_TYPE=Release .. -G "Unix Makefiles"
$ make -j4 install
####### Creating  Debug build #######
$ cd ..
$ mkdir Debug
$ cd Debug
$ cmake -DCMAKE_BUILD_TYPE=Debug .. -G "Unix Makefiles"
$ make -j4 install
```

### Windows \:
*same steps as above for linux , only difference is depending on your environment 
  either Visual Studio or NMake project can be used*
```bash
######### for NMake ####################
cd Release
cmake -DCMAKE_BUILD_TYPE=Release .. -G "NMake Makefiles"
cd ../Debug
cmake -DCMAKE_BUILD_TYPE=Debug   .. -G "NMake Makefiles"
######### for Visual Studio ############
cd Release
cmake -DCMAKE_BUILD_TYPE=Release .. -G "Visual Studio 14 2015"
cd ../Debug
cmake -DCMAKE_BUILD_TYPE=Debug   .. -G "Visual Studio 14 2015"
```
### Mac OS X (Xcode) \:
For the reliable results it is suggested to have the build directory out of source tree.
The process involves creating a build directory, generating an `Xcode` project in that directory with `CMake`,
opening the project file generated in the build directory with `Xcode`, and lastly, adjusting project
settings as requried and kicking off the build.

To generate an `Xcode` project invoke cmake from an empty build directory with command line similar to `cmake -G Xcode -DCMAKE_BUILD_TYPE=Release`.

### Mac OS X (command line tools)
```bash
$ xcode-select --install
$ ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
$ brew doctor
$ brew install cmake
$ brew install boost
$ mkdir Release
$ cd Release
$ cmake -DCMAKE_BUILD_TYPE=Release .. -G "Unix Makefiles"
$ make -j4 install
```

### Parallel Builds
When building on Linux and Mac OS X with Make it's possible to automatically adjust the concurreny of the build using `nproc` command line tool that returns number of CPUs available to the Make execution context\: 

```
$ make -j $(nproc) ... 
```

It's also possible to specifying the target _Load Average_ with `-l` flag to prevent machine overloading\:

```
$ make -l $(nproc) ...
```

And lastly, on Linux, it's possible to run the build with the _BATCH_ scheduling mode (throughput oriented) as\:

```
$ chrt --batch 0 make ...
```

All above considered the ultimate Make invocation combo on Linux would be something like\:

```
$ chrt --batch 0 make -j$(nproc) -l$(nproc)
```

### Running examples\:
#### Testing simple case - single bidder bound to ip/port
- [x] HTTP-Bidder
  * vanilla-rtb/Release/examples/bin$ ./http_bidder_test --config etc/config.cfg
- [x] test with curl and apache benchmark
  * vanilla-rtb/Release/examples/bin$ ./curl.sh --bidder
  * vanilla-rtb/Release/examples/bin$ ./ab.sh -n30000 -c10 --bidder
#### Testing different json mappers to openrtb::BidRequest  available in vanilla-rtb stack
- [x] Start Exchange Handler with HTTP handler
  * vanilla-rtb/Release/examples/bin$ ./exchange_handler_test --config etc/config.cfg
  * vanilla-rtb/Release/examples/bin$ ./ab.sh -n30000 -c10 --auction
  * vanilla-rtb/Release/examples/bin$ ./ab.sh -n30000 -c10 --auction-any
   * vanilla-rtb/Release/examples/bin$ ./ab.sh -n30000 -c10 --auction-rapid
#### Testing mock-bidders e.g. no bid multi bidders via communicator pattern ( work in progress )
- [x] Start Exchange Handler distributing to multi-bidders via communicator 
  * vanilla-rtb/Release/examples/bin$ ./exchange_handler_test --config etc/config.cfg
- [x] Mock-bidders starting multiple in one swoop,  currently configured as 5 bidders in config
  * vanilla-rtb/Release/examples/bin$ ./mock_bidder_test --config etc/config.cfg
  * vanilla-rtb/Release/examples/bin$ ./ab.sh -n30000 -c10 --mock-bidders
#### Testing  multi bidders via communicator pattern ( work in progress )
- [x] multi-bidders starting multiple in one swoop (currently configured as 3 bidders in config ) and starting exchange handler
  * vanilla-rtb/Release/examples/bin$ ./multi_bidder --config etc/config.cfg
  * vanilla-rtb/Release/examples/bin$ ./multi_exchange_handler --config etc/config.cfg
#### Testing cache loader of ad campaigns and ad budgets  ( needs to be extended to get cache update events from outside )
- [x] Cache loader
  * vanilla-rtb/Release/examples/bin$ ./cache_loader_test --config etc/config.cfg
#### Testing Mock exchange writen in python
- [x] Mock exchange - emulating bid requests
  * To run mock exchange you need any python, and python "requests" library installed.
  * for simple exchange please run
  * vanilla-rtb/Release/examples/bin/mock_exchange$ python mock-x.py
  * for various geo and size rotation please run
  * vanilla-rtb/Release/examples/bin/mock_exchange$ python mock-x.py --geo "Russia:Moscow USA:NY USA:Washington USA:Chicago" --size '100:300 240:400 420:280'
  * for more info please run 
  * vanilla-rtb/Release/examples/bin/mock_exchange$ python mock-x.py --help
  
#### Testing campaign manager paired with Win-notification service
- [x] Campaign manager - Budget
  * vanilla-rtb/Release/examples/bin$ ./campaign_manager_test --config etc/config.cfg
- [x] Notification service and Slave-Banker
  * vanilla-rtb/Release/examples/bin$ ./notification_service_test --config etc/config.cfg
  * vanilla-rtb/Release/examples/bin$ ./slavebanker_service_test --config etc/config.cfg
  
- [x] To add/delete/modify campign budgets fire up UI by connecting to manager via browser http:://localhost:11081/campaign/index.html

  ![campaign](https://github.com/venediktov/vanilla-rtb/wiki/images/WorkingBudgetButtons.png)  

