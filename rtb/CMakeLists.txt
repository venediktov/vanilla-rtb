cmake_minimum_required(VERSION 3.1)

project(vanilla-rtb)


set(PACKAGE_NAME "vanilla-rtb"
    CACHE STRING "The packaging name of this library."
   )

set(VANILLA_RTB_VERSION "1.1.0"
    CACHE STRING "The version of the library."
   )


## cConfigure CXX

## Configuring Boost
set(REQUIRED_BOOST_LIBRARIES)
list(APPEND REQUIRED_BOOST_LIBRARIES "log" "serialization" "system")
set(Boost_USE_MULTITHREADED ON)
find_package(Boost
             COMPONENTS "${REQUIRED_BOOST_LIBRARIES}"
             REQUIRED
            )


file(GLOB_RECURSE vanilla_rtb_cpps RELATIVE_PATH "." "*.cpp")
file(GLOB_RECURSE vanilla_rtb_hpps RELATIVE_PATH "." "*.hpp")


if (WIN32)
    # DLLs in Windows appear to have not been fully thought through
    set(DEFAULT_LIBRARY_TYPE "STATIC")
else()
    set(DEFAULT_LIBRARY_TYPE "SHARED")
endif()


#add_library(vanilla_rtb ${DEFAULT_LIBRARY_TYPE} ${vanilla_rtb_cpps} ${vanilla_rtb_hpps} )
add_library(vanilla_rtb ${DEFAULT_LIBRARY_TYPE} ${vanilla_rtb_cpps})

## Configuring vanilla-rtb

target_link_libraries(vanilla_rtb jsonv parsers ${Boost_LIBRARIES})
target_include_directories(vanilla_rtb PUBLIC ${Boost_INCLUDE_DIRS})

# so one can do include "DSL/generic_dsl.hpp", TODO: VL: change RTB to not use broken relative includes and remove this line
target_include_directories(vanilla_rtb PUBLIC "${PROJECT_SOURCE_DIR}")

# so one can do include <rtb/DSL/generic_dsl.hpp>, TODO: VL: improve so no ".." is required
target_include_directories(vanilla_rtb INTERFACE "${PROJECT_SOURCE_DIR}/..")

# so jsonv referred to from rtb headers can be found when rtb headers are used dependent projects, TODO: VL: find out why jsonv transitive dependency does not work and fix so it does
target_include_directories(vanilla_rtb INTERFACE "${PROJECT_SOURCE_DIR}/../jsonv/include")

set_target_properties(vanilla_rtb
                      PROPERTIES
                      SOVERSION ${VANILLA_RTB_VERSION}
                      VERSION   ${VANILLA_RTB_VERSION}
                      LINKER_LANGUAGE CXX
                     )


add_definitions(-DBOOST_LOG_DYN_LINK)

install(TARGETS vanilla_rtb
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

install(FILES ${vanilla_rtb_hpps}
        DESTINATION include/rtb)

