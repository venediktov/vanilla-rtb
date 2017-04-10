cmake_minimum_required(VERSION 3.1)

project(jsmn)


set(PACKAGE_NAME "jsmn"
    CACHE STRING "The packaging name of this library."
   )

set(JSMN_VERSION "1.0.0"
    CACHE STRING "The version of the library."
   )


## Configure CXX
set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE jsmn_cpps RELATIVE_PATH "." "*.c")
file(GLOB_RECURSE jsmn_hpps RELATIVE_PATH "." "*.h")

if (WIN32)
    # DLLs in Windows appear to have not been fully thought through
    set(DEFAULT_LIBRARY_TYPE "STATIC")
else()
    set(DEFAULT_LIBRARY_TYPE "SHARED")
endif()


add_library(jsmn ${DEFAULT_LIBRARY_TYPE} ${jsmn_cpps})

set_target_properties(jsmn
                      PROPERTIES
                      SOVERSION ${JSMN_VERSION}
                      VERSION   ${JSMN_VERSION}
                      LINKER_LANGUAGE CXX
                     )

install(TARGETS jsmn 
  LIBRARY DESTINATION lib
)

