[![alt text][1.1]][1]
[![alt text][2.1]][2]
[![alt text][3.1]][3]
[![alt text][4.1]][4]

[1.1]: https://i.imgur.com/tbK9oQ9.png (twitter icon)
[2.1]: https://i.imgur.com/f1rWtvg.png (LinkedIn icon)
[3.1]: https://i.imgur.com/g8bZTwu.png (telegram icon)
[4.1]: https://i.imgur.com/1slR4ab.png (github icon)

[1]: http://www.twitter.com/vanilla_rtb
[2]: http://www.linkedin.com/company/vanillartb
[3]: http://t.me/vanilla_rtb
[4]: http://www.github.com/vanilla-rtb

# vanilla-rtb

Real Time Bidding (RTB) - Demand Side Platform framework

open-source library utilizing  modern C++11/14  features and latest Boost.

What makes us different from other open-source RTB projects on GitHub :

* Our stack is fairly small and easy to integrate with any cmake project. 
* Partitioned targeting data
* Monolithic bidder 
* Code generation for targeting matchers and bidder executable
* Minimum dependency on outside vendors.
* Decoupled by C++ templates
* Very high throughput up to 105K QPS on 16 cores Intel(R) Xeon(R) CPU E5-2697 v3 @ 2.60GHz
* GUI for editing campaign budget 

As a model cmake project please visit [https://github.com/vanilla-rtb/rapid-bidder](https://github.com/vanilla-rtb/rapid-bidder)

We provide 
[![VanillaRTB extensions](https://img.shields.io/badge/VanillaRTB-extensions-blue.svg)](https://github.com/vanilla-rtb/extensions) including bindings to popular languages **NodeJS/Go/Java/PHP/Python**  
and custom targetings and bidder executable generators [https://github.com/vanilla-rtb/extensions](https://github.com/vanilla-rtb/extensions)

[vanilla-rtb ecosystem](../../wiki)

[Multi-bidder-model-with-communicator-for-Win-notifications](../../wiki/Multi-bidder-model-with-communicator-for-Win-notifications)

[best performance compared to other stacks -  105K QPS](../../wiki/QPS-test)

[runs on cloud with docker - see instructions](../../tree/master/docker/)

[![Join the chat at https://gitter.im/vanilla-rtb/Lobby](https://badges.gitter.im/vanilla-rtb/Lobby.svg)](https://gitter.im/vanilla-rtb/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) 
[![build ](https://travis-ci.org/venediktov/vanilla-rtb.svg?branch=master)](https://travis-ci.org/venediktov/vanilla-rtb)
[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](http://perso.crans.org/besson/LICENSE.html)
[![Installing Dependencies](https://img.shields.io/badge/Dependencies-wiki-green.svg)](https://github.com/venediktov/vanilla-rtb/wiki)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/venediktov/vanilla-rtb/issues)
[![Beerpay](https://beerpay.io/venediktov/vanilla-rtb/badge.svg?style=beer)](https://beerpay.io/venediktov/vanilla-rtb)
[![Backers on Open Collective](https://opencollective.com/vanilla-rtb/backers/badge.svg)](#backers) 
[![Sponsors on Open Collective](https://opencollective.com/vanilla-rtb/sponsors/badge.svg)](#sponsors) 


Recommended build environment: Linux or macOS, CMake >= 3.8, GCC >= 7.0, Clang >= 4.0 , Boost >= 1.64

Structure :
* [/](../../tree/master/) -- the root directory
   * [benchmarks/](../../tree/master/benchmarks/) -- optionaly built benchmarks for IPC caches, json parsers and low overhead IPC audit logger
   * [CRUD/](../../tree/master/CRUD/) -- C++11 high performance HTTP-restful handlers based on boost.ASIO and CRUD API
   * [docker/](../../tree/master/docker/) -- vanilla docker files and instructions on how to build and run 
   * [jsonv/](../../tree/master/jsonv/) -- DSL mapper of json encoded objects to C++ structures
   * [parsers/](../../tree/master/parsers/) -- fast zero copy, zero memory allocation parsers
   * [rapidjson/](../../tree/master/rapidjson/) -- very fast json tokenizer including SAX / DOM API but requires more memory 
   * [rtb/](../../tree/master/rtb/) -- RTB framework
      * [core/](../../tree/master/rtb/core/) -- generic structures shared in the project ( RTB specific )
      * [common/](../../tree/master/rtb/common) -- generic RTB agnostic structures 
      * [datacache/](../../tree/master/rtb/datacache/) -- IPC cache generic classes for fast targeting and other lookups
      * [exchange/](../../tree/master/rtb/exchange) -- exchange handlers implementation
      * [DSL/](../../tree/master/rtb/DSL) --  DSL formats for jsonv , boost::any and rapidjson
   * [examples/](../../tree/master/examples) -- root to our sandbox with examples
      * [bidder/](../../tree/master/examples/bidder) -- collection of application specific classes to support targeting
      * [bidder_experimental/](../../tree/master/examples/bidder_experimental) -- bidder based on chained matchers model
      * [loader/](../../tree/master/examples/loader) -- collection of application specific classes to support campaign loading
      * [campaign/](../../tree/master/examples/campaign) -- add/modify/delete campaign API + UI ( work in progress ) 
      * [datacache/](../../tree/master/examples/datacache) -- IPC cache implementation based on rtb/datacache model 
      * [UI/](../../tree/master/examples/UI) -- HTML and javascript for campaign budget management
      
* [CMakeLists.txt] - cmake file

>The stack of vanilla-rtb includes other C++11 projects and is referencing them via gh-subree.
>To update to the latest version of json-voorhees or CRUD we use the following commands \:

* git subtree pull --prefix jsonv git@github.com:tgockel/json-voorhees.git master --squash
* git subtree pull --prefix CRUD git@github.com:venediktov/CRUD.git  master --squash


### Creating bidder in 49 lines of code
```C++
using Selector = vanilla::ad_selector<vanilla::BudgetManager, Ad>;
using DSLT = DSL::GenericDSL<std::string, DSL::rapid_mapper> ;

//Return from each lambda becomes input for next lambda in the tuple of functions
auto retrieve_domain_f = [&cacheLoader](const std::string& dom, auto&& ...) {
    Domain domain;
    if(!cacheLoader.retrieve(domain,dom)) {
        return boost::optional<uint32_t>();
    }
    return boost::optional<uint32_t>(domain.dom_id);
};

auto retrieve_ico_campaign_f = [&cacheLoader](boost::optional<uint32_t> dom_id, auto&& ...)  {
    std::vector<ICOCampaign> ico_campains;
    if (!cacheLoader.retrieve(ico_campains,*dom_id)) {
        return boost::optional<decltype(ico_campains)>();
    }
    return boost::optional<decltype(ico_campains)>(ico_campains);
};

vanilla::core::Banker<BudgetManager> banker;
auto retrieve_campaign_ads_f = [&](boost::optional<std::vector<ICOCampaign>> campaigns, auto && req, auto && imp)  {
    std::vector<Ad> retrieved_cached_ads;
    for (auto &campaign : *campaigns) {
        if (!cacheLoader.retrieve(retrieved_cached_ads, campaign.campaign_id, imp.banner.get().w, imp.banner.get().h)) {
            continue;
        }
        auto budget_bid = banker.authorize(cacheLoader.get<CampaignCache<BidderConfig>>(), campaign.campaign_id);
        std::transform(std::begin(retrieved_cached_ads),
                       std::end(retrieved_cached_ads),
                       std::begin(retrieved_cached_ads), [budget_bid](Ad & ad){
                          ad.auth_bid_micros = std::min(budget_bid, ad.max_bid_micros);
                          return ad;
                       });
    }
    return retrieved_cached_ads;
};

//creating bidder endpoint utilizing self-referencing pattern
exchange_handler<DSLT> bid_handler(std::chrono::milliseconds(10));
bid_handler
.logger([](const std::string &data) {
    LOG(debug) << "bid request=" << data ;
})
.error_logger([](const std::string &data) {
    LOG(debug) << "bid request error " << data ;
})
.auction_async([&](const BidRequest &request) {
    thread_local vanilla::Bidder<DSLT, Selector> bidder(std::move(Selector()));
    return bidder.bid(request,
                      request.site.get().ref,
                      //chained matchers lambdas defined above
                      retrieve_domain_f,
                      retrieve_ico_campaign_f,
                      retrieve_campaign_ads_f
   );
});
```

### *(&#x1F4D7;) To build vanilla-rtb use following commands in the root of vanilla-rtb*

[(installing dependencies before building vanilla stack)](../../wiki/Installing-dependencies)

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

### Mac OS X (command line tools) \:
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

### Mac OS X ( with llvm ) \:
```bash
$ brew doctor
$ brew install cmake
$ brew install boost
$ brew install --with-clang llvm
$ mkdir Release
$ cd Release
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ -DCMAKE_RANLIB=/usr/local/opt/llvm/bin/llvm-ranlib -DCMAKE_AR=/usr/local/opt/llvm/bin/llvm-ar .. -G "Unix Makefiles"
```
### Parallel Builds
When building on Linux and Mac OS X with Make it's possible to automatically adjust the concurreny of the build using `nproc` for Linux and `sysctl -n hw.physicalcpu` for Mac OS X command line tools that returns number of CPUs available to the Make execution context\: 

Linux :
```
$ make -j $(nproc) ... 
```

Mac OS X :
```
make -j $(sysctl -n hw.physicalcpu) ...
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


## Support on Beerpay
We hope you like our project , fork our repo or chip in for couple of  :beers:!

[![Beerpay](https://beerpay.io/venediktov/vanilla-rtb/badge.svg?style=beer-square)](https://beerpay.io/venediktov/vanilla-rtb)  [![Beerpay](https://beerpay.io/venediktov/vanilla-rtb/make-wish.svg?style=flat-square)](https://beerpay.io/venediktov/vanilla-rtb?focus=wish)

## Contributors

This project exists thanks to all the people who contribute. [[Contribute](CONTRIBUTING.md)].
<a href="../../graphs/contributors"><img src="https://opencollective.com/vanilla-rtb/contributors.svg?width=890&button=false" /></a>


## Backers

Thank you to all our backers! 🙏 [[Become a backer](https://opencollective.com/vanilla-rtb#backer)]

<a href="https://opencollective.com/vanilla-rtb#backers" target="_blank"><img src="https://opencollective.com/vanilla-rtb/backers.svg?width=890"></a>


## Sponsors

Support this project by becoming a sponsor. Your logo will show up here with a link to your website. [[Become a sponsor](https://opencollective.com/vanilla-rtb#sponsor)]

<a href="https://opencollective.com/vanilla-rtb/sponsor/0/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/0/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/1/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/1/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/2/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/2/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/3/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/3/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/4/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/4/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/5/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/5/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/6/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/6/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/7/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/7/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/8/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/8/avatar.svg"></a>
<a href="https://opencollective.com/vanilla-rtb/sponsor/9/website" target="_blank"><img src="https://opencollective.com/vanilla-rtb/sponsor/9/avatar.svg"></a>


