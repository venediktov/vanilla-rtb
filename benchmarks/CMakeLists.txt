CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
SET(CMAKE_CXX_STANDARD 14)

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
            main.cpp)

    TARGET_LINK_LIBRARIES(vanilla-rtb-benchmarks
            benchmark::benchmark
            vanilla_rtb)

    ADD_CUSTOM_TARGET(benchmark
            vanilla-rtb-benchmarks
            DEPENDS vanilla-rtb-benchmarks
            USES_TERMINAL)

    # TODO: VL: remove explicit listings of boost libraries, they should come as transitive dependencies
    FIND_PACKAGE(Boost COMPONENTS "program_options" "log" REQUIRED)
    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
    TARGET_LINK_LIBRARIES(vanilla-rtb-benchmarks ${Boost_LIBRARIES})
ELSE(benchmark_FOUND)
    MESSAGE(STATUS "benchmarks disabled (google.benchmark not found)")

ENDIF(benchmark_FOUND)
