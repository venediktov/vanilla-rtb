# vanilla-rtb
Real Time Bidding (RTB) - Demand Side Platform framework 

Using modern C++11/14  features and latest BOOST libraries 



Structure ( work in progress ) :
* [/](../../tree/master/) -- the root directory
   * [boost-process/](../../tree/master/bost-process/) -- C++11 not in official boost stack
   * [boost-dll/](../../tree/master/boost-dll/) -- C++11 not in official boost stack
   * [rtb/](../../tree/master/rtb/) -- C++11 framework and sandbox for testing platform solution 
      * [core/](../../tree/master/rtb/core/) -- generic structures shared in the project 
      * [datacache/](../../tree/master/rtb/datacache/) -- IPC data store for fast lookups and matching
      * [exchange/](../../tree/master/rtb/exchange) -- exchange handlers implementations ( under review for more generic solution)
      * [DSL/](../../tree/master/rtb/DSL) --  DSL formats for jsonv ( under review )
* [CMakeLists.txt] - cmake file
