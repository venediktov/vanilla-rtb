# vanilla-rtb

Real Time Bidding (RTB) - Demand Side Platform framework 

Utilizing  modern C++11/14  features and latest BOOST libraries some critical path functionality 
written in C and older, easy on the eye,  C++ .

[our model diagram](../../wiki)

[![Join the chat at https://gitter.im/vanilla-rtb/Lobby](https://badges.gitter.im/vanilla-rtb/Lobby.svg)](https://gitter.im/vanilla-rtb/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Structure ( work in progress ) :
* [/](../../tree/master/) -- the root directory
   * [boost-process/](../../tree/master/boost-process/) -- C++11 not in official boost stack
   * [boost-dll/](../../tree/master/boost-dll/) -- C++11 not in official boost stack
   * [CRUD/](../../tree/master/CRUD/) -- Restful web-service written in C++11 based on boost.ASIO and CRUD handlers
   * [rtb/](../../tree/master/rtb/) -- C++11 framework and sandbox for testing platform solution 
      * [core/](../../tree/master/rtb/core/) -- generic structures shared in the project 
      * [datacache/](../../tree/master/rtb/datacache/) -- IPC data store for fast lookups and matching
      * [exchange/](../../tree/master/rtb/exchange) -- exchange handlers implementations ( under review for more generic solution)
      * [DSL/](../../tree/master/rtb/DSL) --  DSL formats for jsonv ( under review )
* [CMakeLists.txt] - cmake file

>The stack of vanilla-rtb depends on other C++11 projects and is referencing them via gh-subree.
>To update to the latest version of boost-process , boost-dll , json-voorhees or CRUD  use the following commands \:

* git subtree pull --prefix jsonv git@github.com:tgockel/json-voorhees.git master --squash
* git subtree pull --prefix boost-process git@github.com:BorisSchaeling/boost-process.git master --squash
* git subtree pull --prefix boost-dll git@github.com:apolukhin/Boost.DLL.git master --squash
* git subtree pull --prefix CRUD git@github.com:venediktov/CRUD.git  master --squash



###*(&#x1F4D7;) To build vanilla-rtb use following commands in the root of vanilla-rtb*

###Linux \:
- [x] mkdir Release
- [x] cd Release
- [x] cmake -DCMAKE_BUILD_TYPE=Release .. -G "Unix Makefiles"
- [x] gmake VERBOSE=1
- [x] cd ..
- [x] mkdir Debug
- [x] cd Debug
- [x] cmake -DCMAKE_BUILD_TYPE=Debug .. -G "Unix Makefiles"
- [x] gmake VERBOSE=1

###Windows \:
*same steps as above for linux , only difference is depending on your environment 
  either Visual Studio or NMake project can be used*
  
- [x] cmake -DCMAKE_BUILD_TYPE=Release .. -G "NMake Makefiles"
- [x] cmake -DCMAKE_BUILD_TYPE=Debug   .. -G "NMake Makefiles"
- [x] cmake -DCMAKE_BUILD_TYPE=Release .. -G "Visual Studio 14 2015"
- [x] cmake -DCMAKE_BUILD_TYPE=Debug   .. -G "Visual Studio 14 2015"


###For faster builds invoking multiple make processes  , find number of cores on your system
Linux command \: 
* nproc

4

pass it to your make script like this
gmake -j4
