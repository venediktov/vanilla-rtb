cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

set(REQUIRED_BOOST_LIBRARIES)
list(APPEND REQUIRED_BOOST_LIBRARIES "log" "program_options" "system" "serialization")
set(Boost_USE_MULTITHREADED ON)
find_package(Boost
             COMPONENTS "${REQUIRED_BOOST_LIBRARIES}"
             REQUIRED
            )


if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

## Configuring vanilla-rtb
include_directories("${PROJECT_SOURCE_DIR}"
                    "${PROJECT_SOURCE_DIR}/rtb"
                    "${PROJECT_SOURCE_DIR}/jsonv/include"
                   )

add_definitions(-DBOOST_LOG_DYN_LINK)


if (WIN32)
    link_directories(${Boost_LIBRARY_DIRS})
endif(WIN32)

add_executable(
        exchange_handler_test
        exchange_handler_test.cpp
        )

add_executable(
        country_cache_test
        country_cache_test.cpp
        )

if (WIN32)
    target_compile_definitions(exchange_handler_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(jsonv PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
endif(WIN32)

find_package (Threads)


target_link_libraries(
        exchange_handler_test
        crud_service
        vanilla_rtb
        jsonv
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )


target_link_libraries(
        country_cache_test
        vanilla_rtb
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        rt
        )

