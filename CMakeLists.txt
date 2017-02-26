# -*- cmake -*-

cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -ggdb")

add_subdirectory(jsonv)
add_subdirectory(CRUD)
add_subdirectory(rtb)
add_subdirectory(examples)
