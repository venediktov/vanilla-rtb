cmake_minimum_required(VERSION 3.1)


include_directories(
                   "${PROJECT_SOURCE_DIR}/examples"
                   "${PROJECT_SOURCE_DIR}/examples/bidder"
                   )

add_executable(
    ico_campaign_bidder_test
    ico_campaign_bidder_test.cpp
)

if (WIN32) 
    target_compile_definitions(ico_campaign_bidder_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(jsonv PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
endif(WIN32)

target_link_libraries(
    ico_campaign_bidder_test
    crud_service
    vanilla_rtb
    jsonv
    parsers
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${RT_LIB}
)

install(TARGETS ico_campaign_bidder_test
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

#TODO
#add_custom_target(
#    generator
#    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/generator && python ico.py && python campaign_budget.py ico > ../data/ico_campaign_budget
#)

#add_dependencies(ico_campaign_bidder_test generator)

