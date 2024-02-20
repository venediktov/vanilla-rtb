# File: CMakeLists used for generating a Makefile (or whatever your build system is).
#
# Copyright 2015 by Travis Gockel
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
# an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.
cmake_minimum_required(VERSION 3.20)

project(json-voorhees)

set(PACKAGE_NAME "jsonv"
    CACHE STRING "The packaging name of this library."
   )

################################################################################
# Configuration                                                                #
################################################################################

# Extract version information from the config file. It can still be overridden by the user setting JSONV_VERSION, but
# that is probably a bad idea.
file(READ include/jsonv/config.hpp JSONV_CONFIG_H_STR)
string(REGEX REPLACE ".*#define +JSONV_VERSION_MAJOR +([0-9]+).*" "\\1" JSONV_VERSION_MAJOR "${JSONV_CONFIG_H_STR}")
string(REGEX REPLACE ".*#define +JSONV_VERSION_MINOR +([0-9]+).*" "\\1" JSONV_VERSION_MINOR "${JSONV_CONFIG_H_STR}")
string(REGEX REPLACE ".*#define +JSONV_VERSION_PATCH +([0-9]+).*" "\\1" JSONV_VERSION_PATCH "${JSONV_CONFIG_H_STR}")

set(JSONV_VERSION "${JSONV_VERSION_MAJOR}.${JSONV_VERSION_MINOR}.${JSONV_VERSION_PATCH}"
    CACHE STRING "The version of the library."
   )
message(STATUS "JSONV version: ${JSONV_VERSION}")

option(JSONV_BUILD_TESTS
       "Should the unit test library get built?"
       OFF
      )

set(REQUIRED_BOOST_LIBRARIES)

if (JSONV_BUILD_TESTS)
    list(APPEND REQUIRED_BOOST_LIBRARIES "filesystem" "system")
endif()

#######################
# Regex Configuration #
#######################

option(USE_BOOST_REGEX
       "Controls the variable JSONV_REGEX_USE_BOOST (see C++ documentation)."
       OFF
      )
if (USE_BOOST_REGEX)
    add_definitions("-DJSONV_REGEX_USE_BOOST=1")
    list(APPEND REQUIRED_BOOST_LIBRARIES "regex")
endif(USE_BOOST_REGEX)

#############################
# String View Configuration #
#############################

option(USE_BOOST_STRING_VIEW
       "Controls the variable JSONV_STRING_VIEW_USE_BOOST (see C++ documentation)."
       OFF
      )
if (USE_BOOST_STRING_VIEW)
    add_definitions("-DJSONV_STRING_VIEW_USE_BOOST=1")
endif(USE_BOOST_STRING_VIEW)

option(USE_STD_STRING_VIEW
       "Controls the variable JSONV_STRING_VIEW_USE_STD (see C++ documentation)."
       OFF
      )
if (USE_STD_STRING_VIEW)
    add_definitions("-DJSONV_STRING_VIEW_USE_STD=1")
endif(USE_STD_STRING_VIEW)

##########################
# Optional Configuration #
##########################

option(USE_STD_OPTIONAL
       "Controls the variable JSONV_OPTIONAL_USE_STD (see C++ documentation)."
       OFF
      )
if (USE_STD_OPTIONAL)
    add_definitions("-DJSONV_OPTIONAL_USE_STD=1")
endif()

option(USE_STD_EXPERIMENTAL_OPTIONAL
       "Controls the variable JSONV_OPTIONAL_USE_STD_EXPERIMENTAL (see C++ documentation)."
       OFF
      )
if (USE_STD_EXPERIMENTAL_OPTIONAL)
    add_definitions("-DJSONV_OPTIONAL_USE_STD_EXPERIMENTAL=1")
endif()

option(USE_BOOST_OPTIONAL
       "Controls the variable JSONV_OPTIONAL_USE_BOOST (see C++ documentation)."
       OFF
      )
if (USE_BOOST_OPTIONAL)
    add_definitions("-DJSONV_OPTIONAL_USE_BOOST=1")
endif()

########################
# Locale Configuration #
########################

if (CMAKE_COMPILER_IS_GNUCC)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0")
        set(USE_BOOST_LOCALE_DEFAULT ON)
    else()
        set(USE_BOOST_LOCALE_DEFAULT OFF)
    endif()
else()
    set(USE_BOOST_LOCALE_DEFAULT OFF)
endif()

option(USE_BOOST_LOCALE
       "Should Boost.Locale be used for character encoding conversions (as opposed to the standard library)?"
       ${USE_BOOST_LOCALE_DEFAULT}
      )
if (USE_BOOST_LOCALE)
    add_definitions("-DJSONV_CHAR_CONVERT_USE_BOOST_LOCALE=1")
    list(APPEND REQUIRED_BOOST_LIBRARIES "locale")
else()
    add_definitions("-DJSONV_CHAR_CONVERT_USE_BOOST_LOCALE=0")
endif()

option(COVERAGE
       "Enable building with code coverage."
       OFF
      )
if (COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif(COVERAGE)

if (WIN32)
    # DLLs in Windows appear to have not been fully thought through
    set(DEFAULT_LIBRARY_TYPE "STATIC")
else()
    set(DEFAULT_LIBRARY_TYPE "SHARED")
endif()

set(LIBRARY_TYPE ${DEFAULT_LIBRARY_TYPE}
    CACHE STRING "The type of library to build (SHARED or STATIC)"
   )

option(BENCHMARK
       "Enable compilation of the json-benchmark program."
       OFF
      )

if(WIN32)
else(WIN32)
    # Reasonable compilers...
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -ggdb -DJSONV_DEBUG=1")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ggdb")
endif(WIN32)

include_directories("${PROJECT_SOURCE_DIR}/include"
                    "${PROJECT_SOURCE_DIR}/src"
                   )

set(Boost_USE_MULTITHREADED ON)
find_package(Boost
             "${RTB_BOOST_VERSION}"
             COMPONENTS "${REQUIRED_BOOST_LIBRARIES}"
             REQUIRED
            )
include_directories(${Boost_INCLUDE_DIRS})

add_definitions("-DJSONV_TEST_DATA_DIR=\"${CMAKE_SOURCE_DIR}/src/jsonv-tests/data\"")

configure_file(libjsonv.pc.in libjsonv.pc)

################################################################################
# Building                                                                     #
################################################################################

file(GLOB_RECURSE jsonv_cpps RELATIVE_PATH "." "src/jsonv/*.cpp")
add_library(jsonv ${LIBRARY_TYPE} ${jsonv_cpps})
set_target_properties(jsonv
                      PROPERTIES
                          SOVERSION ${JSONV_VERSION}
                          VERSION   ${JSONV_VERSION}
                     )

if (Boost_LIBRARIES)
    target_link_libraries(jsonv ${Boost_LIBRARIES})
endif()

if (JSONV_BUILD_TESTS)
    file(GLOB_RECURSE jsonv_tests_cpps RELATIVE_PATH "." "src/jsonv-tests/*.cpp")
    add_executable(jsonv-tests ${jsonv_tests_cpps})
    target_link_libraries(jsonv-tests
            "jsonv"
            ${Boost_LIBRARIES}
        )
    
    add_custom_target(check
                      COMMAND $<TARGET_FILE:jsonv-tests>
                      DEPENDS jsonv-tests
                      USES_TERMINAL
                     )
endif()

target_compile_options(jsonv INTERFACE -Wno-deprecated-declarations)


if (COVERAGE)
    target_link_libraries(jsonv "gcov")
    if (JSONV_BUILD_TESTS)
        target_link_libraries(jsonv-tests "gcov")
    endif()
endif(COVERAGE)

################################################################################
# Benchmarking Tool                                                            #
################################################################################

option(BENCHMARK_JSONV
       "Enable benchmark suite for JSON Voorhees?"
       YES
      )

option(BENCHMARK_JSONCPP
       "Enable benchmark suite for JsonCpp?"
       YES
      )

option(BENCHMARK_JQ
       "Enable benchmark suite for jq?"
       NO
      )

option(BENCHMARK_JANSSON
       "Enable benchmark suite for jansson?"
       NO
      )

if (BENCHMARK)
    set(BENCHMARK_CPPS src/json-benchmark/core.cpp src/json-benchmark/main.cpp)
    set(BENCHMARK_LIBS "")
    macro(add_benchmark_suite CPP_FILE LIB_NAME)
        list(APPEND BENCHMARK_CPPS "src/json-benchmark/${CPP_FILE}")
        list(APPEND BENCHMARK_LIBS "${LIB_NAME}")
    endmacro()

    if (BENCHMARK_JSONV)
        add_benchmark_suite("jsonv_benchmark.cpp" "jsonv")
    endif()

    if (BENCHMARK_JSONCPP)
        add_benchmark_suite("jsoncpp_benchmark.cpp" "jsoncpp")
        include_directories("/usr/include/jsoncpp")
    endif()

    if (BENCHMARK_JQ)
        add_benchmark_suite("jq_benchmark.cpp" "jq")
    endif()

    if (BENCHMARK_JANSSON)
        add_benchmark_suite("jansson_benchmark.cpp" "jansson")
    endif()

    add_executable(json-benchmark ${BENCHMARK_CPPS})
    target_link_libraries(json-benchmark
        ${BENCHMARK_LIBS}
        ${Boost_LIBRARIES}
    )
endif(BENCHMARK)

################################################################################
# Installation                                                                 #
################################################################################

# directories
if (OS_WINDOWS)
    set(RUNTIME_INSTALL_DIR Bin)
    set(INCLUDE_INSTALL_DIR Include)
    set(LIBRARY_INSTALL_DIR Lib)
    set(CONFIG_INSTALL_DIR  CMake)
else()
    set(RUNTIME_INSTALL_DIR bin)
    set(INCLUDE_INSTALL_DIR include)
    set(LIBRARY_INSTALL_DIR_DEFAULT lib)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIBRARY_INSTALL_DIR_DEFAULT lib64)
    endif ()
    set(LIBRARY_INSTALL_DIR ${LIBRARY_INSTALL_DIR_DEFAULT}
        CACHE STRING "Directory of installed libraries, e.g., \"lib64\""
       )
    mark_as_advanced(LIBRARY_INSTALL_DIR)
    set(CONFIG_INSTALL_DIR  "${LIBRARY_INSTALL_DIR}/cmake/${PACKAGE_NAME}")
endif ()

foreach(area RUNTIME LIBRARY INCLUDE CONFIG)
    if (NOT IS_ABSOLUTE "${${area}_INSTALL_DIR}")
        set(${area}_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${${area}_INSTALL_DIR}")
    endif()
    message(STATUS "${area}_INSTALL_DIR == ${${area}_INSTALL_DIR}")
endforeach()

install(TARGETS jsonv
        EXPORT  exported
        LIBRARY DESTINATION "${LIBRARY_INSTALL_DIR}"
        ARCHIVE DESTINATION "${LIBRARY_INSTALL_DIR}"
       )
install(DIRECTORY   "include/jsonv"
        DESTINATION "${INCLUDE_INSTALL_DIR}"
       )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libjsonv.pc DESTINATION "${LIBRARY_INSTALL_DIR}/pkgconfig")
install(EXPORT exported DESTINATION "${CONFIG_INSTALL_DIR}" FILE "${PACKAGE_NAME}-export.cmake")

################################################################################
# Packaging                                                                    #
################################################################################

set(PACKAGE_SYSTEM ""
    CACHE STRING "The packaging system to generate a package build file for (leave blank to not build a package)"
   )
if (PACKAGE_SYSTEM)
    message(STATUS "Creating packaging for ${PACKAGE_SYSTEM}...")
    set(CPACK_PACKAGE_VERSION "${JSONV_VERSION}")
    set(CPACK_GENERATOR "${PACKAGE_SYSTEM}")
    set(CPACK_PACKAGE_NAME "${PACKAGE_NAME}")
    set(CPACK_PACKAGE_RELEASE 1)
    set(CPACK_PACKAGE_CONTACT "Travis Gockel")
    set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
    if (PACKAGE_SYSTEM STREQUAL "RPM")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/installer/rpm/jsonv.spec.in"
                    "${CMAKE_CURRENT_BINARY_DIR}/installer/rpm/jsonv.spec"
                    @ONLY IMMEDIATE
                    )
        set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/installer/rpm/jsonv.spec")
    else ()
        message(WARNING "Unknown PACKAGE_SYSTEM=${PACKAGE_SYSTEM} -- anything could happen with your package")
    endif ()
    include(CPack)
endif ()
