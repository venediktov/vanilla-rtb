cmake_minimum_required(VERSION 3.1)

set(REQUIRED_BOOST_LIBRARIES)
list(APPEND REQUIRED_BOOST_LIBRARIES "log" "program_options" "system" "serialization" "date_time")
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
                    "${PROJECT_SOURCE_DIR}/jsonv/include"
                   )

add_definitions(-DBOOST_LOG_DYN_LINK -DBOOST_PROGRAM_OPTIONS_DYN_LINK)


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

add_executable(
        mock_bidder_test
        mock_bidder_test.cpp
        )

add_executable(
        messaging_client_test
        messaging_client_test.cpp
        )

if (WIN32)
    target_compile_definitions(exchange_handler_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(messaging_client_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(mock_bidder_test PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1 _CRT_NONSTDC_NO_DEPRECATE )
    target_compile_definitions(jsonv PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
endif(WIN32)

find_package (Threads)


target_link_libraries(
        exchange_handler_test
        crud_service
        vanilla_rtb
        jsonv
        parsers
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

if (NOT WIN32 AND NOT APPLE)
set(RT_LIB rt)
endif()

target_link_libraries(
        country_cache_test
        vanilla_rtb
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${RT_LIB}
        )

target_link_libraries(
        mock_bidder_test
        vanilla_rtb
        jsonv
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${RT_LIB}
        )

target_link_libraries(
        messaging_client_test
        vanilla_rtb
        jsonv
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${RT_LIB}
        )

install(TARGETS exchange_handler_test messaging_client_test mock_bidder_test country_cache_test
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

add_subdirectory(bidder)
add_subdirectory(loader)
add_subdirectory(campaign)
add_subdirectory(multiexchange)
add_subdirectory(bidder_experimental)

#Install all json files from our examples
file(GLOB_RECURSE JSON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.json)
INSTALL(FILES ${JSON_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

#Install all shell scripts running our samples
file(GLOB_RECURSE SHELL_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
INSTALL(FILES ${SHELL_SCRIPTS}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/etc DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bidder/data DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/UI/CampaignBudgetsCDN/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/www/campaign)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bidder/mock_exchange/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/mock_exchange)


