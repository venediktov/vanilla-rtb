CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

IF(benchmark_DIR)
    FIND_PACKAGE(benchmark PATHS "${benchmark_DIR}" QUIET)
ELSE(benchmark_DIR)
    FIND_PACKAGE(benchmark QUIET)
ENDIF(benchmark_DIR)

IF(benchmark_FOUND)
    MESSAGE(STATUS "benchmarks enabled (using google.benchmark ${benchmark_VERSION} from ${benchmark_DIR})")

    ADD_EXECUTABLE(
            vanilla-rtb-benchmarks
            rtb_dsl_benchmarks.cpp
            rtb_cache_benchmarks.cpp
            audit_benchmarks.cpp
            main.cpp)

    TARGET_SOURCES(vanilla-rtb-benchmarks PRIVATE
        audit_buffer.hpp
        audit_codec.hpp)

    TARGET_LINK_LIBRARIES(vanilla-rtb-benchmarks
            benchmark::benchmark
            vanilla_rtb)

    ADD_CUSTOM_TARGET(benchmark
            vanilla-rtb-benchmarks
            DEPENDS vanilla-rtb-benchmarks generator
            USES_TERMINAL)

    # copies the configuration file from vanilla examples required by some benchmarks
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/examples/etc/config.cfg ${CMAKE_CURRENT_BINARY_DIR}/config.cfg COPYONLY)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/examples/bidder/data/ads ${CMAKE_CURRENT_BINARY_DIR}/data/ads COPYONLY)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/examples/bidder/data/geo ${CMAKE_CURRENT_BINARY_DIR}/data/geo COPYONLY)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/examples/bidder/data/geo_campaign ${CMAKE_CURRENT_BINARY_DIR}/data/geo_campaign COPYONLY)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/examples/bidder/data/campaign_budget ${CMAKE_CURRENT_BINARY_DIR}/data/campaign_budget COPYONLY)

    # FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)

    # ADD_CUSTOM_COMMAND(
    #     TARGET vanilla-rtb-benchmarks
    #     PRE_BUILD
    #     BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/ads ${CMAKE_CURRENT_BINARY_DIR}/geo ${CMAKE_CURRENT_BINARY_DIR}/geo_campaign
    #     COMMAND ${CMAKE_SOURCE_DIR}/examples/bidder/generator/ad.py ${CMAKE_CURRENT_BINARY_DIR}
    # )

    # TODO: VL: remove explicit listings of boost libraries, they should come as transitive dependencies
    FIND_PACKAGE(Boost COMPONENTS "program_options" "log" REQUIRED)
    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
    TARGET_LINK_LIBRARIES(vanilla-rtb-benchmarks ${Boost_LIBRARIES})
ELSE(benchmark_FOUND)
    MESSAGE(STATUS "benchmarks disabled (google.benchmark not found)")

ENDIF(benchmark_FOUND)
