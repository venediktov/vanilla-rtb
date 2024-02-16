# -*- cmake -*-

cmake_minimum_required(VERSION 3.3.2)

include_directories(${Boost_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})

add_executable(
        regex_restful_service
        regex_restful_service.cpp
        )

add_executable(
        simple_restful_service
        simple_restful_service.cpp
        )

add_executable(
        persisted_regex_restful_service
        persisted_regex_restful_service.cpp
        )

add_executable(
        webserver 
        webserver.cpp
        )

find_package (Threads)

target_link_libraries(
        regex_restful_service
        crud_service
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

target_link_libraries(
        simple_restful_service
        crud_service
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

target_link_libraries(
        persisted_regex_restful_service
        crud_service
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )


target_link_libraries(
        webserver 
        crud_service
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

