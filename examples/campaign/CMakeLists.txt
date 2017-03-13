cmake_minimum_required(VERSION 3.1)


include_directories(
                   "${PROJECT_SOURCE_DIR}/examples"
                   "${PROJECT_SOURCE_DIR}/examples/bidder"
                   )

add_executable(
    campaign_manager_test
    campaign_manager_test.cpp
)


if (WIN32)
    target_compile_definitions(campaign_manager_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(jsonv PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
endif(WIN32)

target_link_libraries(
    campaign_manager_test
    crud_service
    vanilla_rtb
    jsonv
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${RT_LIB}
)
