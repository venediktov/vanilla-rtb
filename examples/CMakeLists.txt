cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

set(REQUIRED_BOOST_LIBRARIES)
list(APPEND REQUIRED_BOOST_LIBRARIES "log" )
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

add_executable(
        exchange_handler_test
        exchange_handler_test.cpp
        )

target_link_libraries(
        exchange_handler_test
        crud_service
        vanilla_rtb
        jsonv
        ${Boost_LIBRARIES}
        pthread
        )



