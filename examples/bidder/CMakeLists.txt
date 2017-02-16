cmake_minimum_required(VERSION 3.1)

add_executable(
    http_bidder_test
    http_bidder_test.cpp
)

target_link_libraries(
    http_bidder_test
    crud_service
    vanilla_rtb
    jsonv
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${RT_LIB}
)
