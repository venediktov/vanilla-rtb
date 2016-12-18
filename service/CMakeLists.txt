
set(CMAKE_CXX_STANDARD 14)

## Configuring Boost
set(REQUIRED_BOOST_LIBRARIES)
list(APPEND REQUIRED_BOOST_LIBRARIES "system")
list(APPEND REQUIRED_BOOST_LIBRARIES "regex")
set(Boost_USE_MULTITHREADED ON)

find_package(Boost
             COMPONENTS "${REQUIRED_BOOST_LIBRARIES}"
             REQUIRED
            )

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

include_directories(../handlers)

if (WIN32)
    # DLLs in Windows appear to have not been fully thought through
    set(DEFAULT_LIBRARY_TYPE "STATIC")
else()
    set(DEFAULT_LIBRARY_TYPE "SHARED")
endif()

add_library(crud_service
            ${DEFAULT_LIBRARY_TYPE}
	    mime_types.cpp
	    reply.cpp
	    request_handler.cpp
	    request_parser.cpp
           )

if (Boost_LIBRARIES)
    target_link_libraries( crud_service ${Boost_LIBRARIES} pthread)
endif()

