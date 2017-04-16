CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
SET(CMAKE_CXX_STANDARD 14)

FIND_PACKAGE(benchmark PATHS /home/bobah/pkg/tp/benchmark-312d9d0 REQUIRED)

IF(benchmark_FOUND)
    MESSAGE(STATUS "benchmarks enabled (using google.benchmark ${benchmark_VERSION} from ${benchmark_DIR})")

    ADD_EXECUTABLE(
            vanilla-rtb-benchmarks
            rtb_dsl_benchmarks.cpp
            main.cpp)

    TARGET_LINK_LIBRARIES(vanilla-rtb-benchmarks
            benchmark::benchmark
            vanilla_rtb)

    ADD_CUSTOM_TARGET(benchmark
            vanilla-rtb-benchmarks
            DEPENDS vanilla-rtb-benchmarks
            USES_TERMINAL)

ELSE(benchmark_FOUND)
    MESSAGE(STATUS "benchmarks disabled (google.benchmark not found)")

ENDIF(benchmark_FOUND)
