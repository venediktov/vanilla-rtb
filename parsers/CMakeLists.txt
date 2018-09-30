cmake_minimum_required(VERSION 3.1)

project(parsers)


set(PACKAGE_NAME "parsers"
    CACHE STRING "The packaging name of this library."
   )

set(JSMN_VERSION "1.0.0"
    CACHE STRING "The version of the library."
   )


## Configure CXX

file(GLOB_RECURSE parsers_cs RELATIVE_PATH "." "*.c")
file(GLOB_RECURSE parsers_hs RELATIVE_PATH "." "*.h")
file(GLOB_RECURSE parsers_cpps RELATIVE_PATH "." "*.cpp")
file(GLOB_RECURSE parsers_hpps RELATIVE_PATH "." "*.hpp")

if (WIN32)
    # DLLs in Windows appear to have not been fully thought through
    set(DEFAULT_LIBRARY_TYPE "STATIC")
else()
    set(DEFAULT_LIBRARY_TYPE "SHARED")
endif()


add_library(parsers ${DEFAULT_LIBRARY_TYPE} ${parsers_cs} ${parsers_cpps})

set_target_properties(parsers
                      PROPERTIES
                      SOVERSION ${JSMN_VERSION}
                      VERSION   ${JSMN_VERSION}
                      LINKER_LANGUAGE CXX
                     )

install(TARGETS parsers 
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(FILES ${parsers_hpps} ${parsers_hs}
        DESTINATION include/parsers)
