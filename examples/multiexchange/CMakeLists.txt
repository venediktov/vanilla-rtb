cmake_minimum_required(VERSION 3.1)

add_executable(
    multi_exchange_handler
    multi_exchange_handler.cpp
)

if (WIN32) 
    target_compile_definitions(multi_exchange_handler PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
    target_compile_definitions(jsonv PRIVATE JSON_SO=1 _LIB JSON_COMPILING=1)
endif(WIN32)

target_link_libraries(
        multi_exchange_handler
        crud_service
        vanilla_rtb
        jsonv
        parsers
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

install(TARGETS multi_exchange_handler
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

