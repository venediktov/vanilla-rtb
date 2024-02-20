include_directories(
                   "${PROJECT_SOURCE_DIR}/examples"
                   "${PROJECT_SOURCE_DIR}/examples/bidder"
                   )

add_executable(
    cache_loader_test
    cache_loader_test.cpp
)

add_executable(
    ico_campaign_loader_test
    ico_campaign_loader_test.cpp
)



if (WIN32)
    target_compile_definitions(cache_loader_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(jsonv PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
endif(WIN32)

target_link_libraries(
    cache_loader_test
    crud_service
    vanilla_rtb
    jsonv
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${RT_LIB}
)

target_link_libraries(
    ico_campaign_loader_test
    crud_service
    vanilla_rtb
    jsonv
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${RT_LIB}
)

install(TARGETS cache_loader_test ico_campaign_loader_test
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

