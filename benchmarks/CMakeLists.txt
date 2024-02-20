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

    FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/../examples/etc/config.cfg ${CMAKE_CURRENT_BINARY_DIR}/config.cfg COPYONLY)
    
    MESSAGE(STATUS "Benchmark dir for generated data: ${CMAKE_CURRENT_BINARY_DIR}/data")
    ADD_CUSTOM_COMMAND(
        TARGET vanilla-rtb-benchmarks
        PRE_BUILD
        BYPRODUCTS
          ${CMAKE_CURRENT_BINARY_DIR}/data/ads 
          ${CMAKE_CURRENT_BINARY_DIR}/data/geo
          ${CMAKE_CURRENT_BINARY_DIR}/data/geo_campaign
          ${CMAKE_CURRENT_BINARY_DIR}/data/campaign_budget
        COMMAND
          cd ${CMAKE_CURRENT_BINARY_DIR}/data &&
          ${CMAKE_CURRENT_SOURCE_DIR}/../examples/bidder/generator/ad.py &&
          ${CMAKE_CURRENT_SOURCE_DIR}/../examples/bidder/generator/campaign_budget.py > ${CMAKE_CURRENT_BINARY_DIR}/data/campaign_budget
    )

    # TODO: VL: remove explicit listings of boost libraries, they should come as transitive dependencies
    FIND_PACKAGE(Boost COMPONENTS "program_options" "log" REQUIRED)
    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
    TARGET_LINK_LIBRARIES(vanilla-rtb-benchmarks ${Boost_LIBRARIES})
ELSE(benchmark_FOUND)
    MESSAGE(STATUS "benchmarks disabled (google.benchmark not found)")

ENDIF(benchmark_FOUND)
