cmake_minimum_required(VERSION 3.1)

add_executable(
    http_bidder_test
    http_bidder_test.cpp
)

add_executable(
    multi_exchange_handler
    multi_exchange_handler.cpp
)

add_executable(
    multi_bidder
    multi_bidder.cpp
)

if (WIN32) 
    target_compile_definitions(http_bidder_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(multi_exchange_handler PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(multi_bidder PRIVATE  JSON_SO=1 _LIB JSON_COMPILING=1)
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

target_link_libraries(
        multi_exchange_handler
        crud_service
        vanilla_rtb
        jsonv
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

target_link_libraries(
        multi_bidder
        crud_service
        vanilla_rtb
        jsonv
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${RT_LIB}
        )
