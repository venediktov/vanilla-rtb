cmake_minimum_required(VERSION 3.1)

project(vanilla-rtb)


set(PACKAGE_NAME "vanilla-rtb"
    CACHE STRING "The packaging name of this library."
   )

set(VANILLA_RTB_VERSION "1.1.0"
    CACHE STRING "The version of the library."
   )


## cConfigure CXX
set(CMAKE_CXX_STANDARD 14)

## Configuring Boost
set(REQUIRED_BOOST_LIBRARIES)
list(APPEND REQUIRED_BOOST_LIBRARIES "log")
set(Boost_USE_MULTITHREADED ON)
find_package(Boost
             COMPONENTS "${REQUIRED_BOOST_LIBRARIES}"
             REQUIRED
            )

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()


## Configuring vanilla-rtb
include_directories("${PROJECT_SOURCE_DIR}/core"
                    "${PROJECT_SOURCE_DIR}/datacache"
                    "${PROJECT_SOURCE_DIR}/DSL"
                    "${PROJECT_SOURCE_DIR}/exchange"
                   )

file(GLOB_RECURSE vanilla_rtb_cpps RELATIVE_PATH "." "*.cpp")
file(GLOB_RECURSE vanilla_rtb_hpps RELATIVE_PATH "." "*.hpp")


if (WIN32)
    # DLLs in Windows appear to have not been fully thought through
    set(DEFAULT_LIBRARY_TYPE "STATIC")
else()
    set(DEFAULT_LIBRARY_TYPE "SHARED")
endif()


add_library(vanilla_rtb ${DEFAULT_LIBRARY_TYPE} ${vanilla_rtb_cpps} ${vanilla_rtb_hpps} )

set_target_properties(vanilla_rtb
                      PROPERTIES
                      SOVERSION ${VANILLA_RTB_VERSION}
                      VERSION   ${VANILLA_RTB_VERSION}
                      LINKER_LANGUAGE CXX
                     )


if (Boost_LIBRARIES)
    target_link_libraries(vanilla_rtb PUBLIC ${Boost_LIBRARIES})
endif()

