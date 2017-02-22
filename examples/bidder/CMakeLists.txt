cmake_minimum_required(VERSION 3.1)

add_executable(
    http_bidder_test
    http_bidder_test.cpp
)

if (WIN32)
    target_compile_definitions(http_bidder_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(jsonv PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
endif(WIN32)

target_link_libraries(
    http_bidder_test
    crud_service
    vanilla_rtb
    jsonv
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${RT_LIB}
)
