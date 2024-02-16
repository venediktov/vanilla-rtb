
set(Boost_USE_MULTITHREADED ON)
find_package(Boost "${CRUD_BOOST_VERSION}" COMPONENTS "log" "system" "serialization" "program_options" REQUIRED)

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

