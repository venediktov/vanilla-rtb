# -*- cmake -*-

cmake_minimum_required(VERSION 3.3.2)

# VERSION not required, but good for library versioning
project(CRUD VERSION 0.1.0)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(service)

set(CMAKE_CXX_STANDARD 14)

set ( CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost
             COMPONENTS "log" "system" "serialization" "program_options"
             REQUIRED
            )

include_directories(${Boost_INCLUDE_DIRS} handlers service)

add_executable(
        my_service
        main.cpp
        )

find_package (Threads)

target_link_libraries(
        my_service
        crud_service
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

