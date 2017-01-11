# -*- cmake -*-

cmake_minimum_required(VERSION 2.8)

add_subdirectory(service)

set(CMAKE_CXX_STANDARD 14)

include_directories(${Boost_INCLUDE_DIRS} handlers service)

add_executable(
        my_service
        main.cpp
        )

find_package (Threads)

target_link_libraries(
        my_service
        crud_service
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

