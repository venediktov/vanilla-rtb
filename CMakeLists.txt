# -*- cmake -*-

cmake_minimum_required(VERSION 2.8)

project(vanilla-rtb)

if(WIN32)
else(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -ggdb")
endif(WIN32)


add_subdirectory(jsonv)
add_subdirectory(CRUD)
add_subdirectory(rtb)
add_subdirectory(redisclient)
add_subdirectory(examples)
