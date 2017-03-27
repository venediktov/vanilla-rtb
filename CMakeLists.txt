# -*- cmake -*-

cmake_minimum_required(VERSION 2.8)


project(vanilla-rtb)

if(WIN32)
else(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -ggdb")
endif(WIN32)


set ( CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/examples )
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_CURRENT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}")

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib64")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib64")
ENDIF("${isSystemDir}" STREQUAL "-1")

#file(COPY
#  ${CMAKE_CURRENT_BINARY_DIR}/CRUD/service/libcrud_service.so
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#  FILE_PERMISSIONS
#    OWNER_READ OWNER_WRITE OWNER_EXECUTE
#    GROUP_READ
#    WORLD_READ
#)

file(GLOB CRUD_LIB ${CMAKE_CURRENT_BINARY_DIR}/CRUD/service/*.so)
INSTALL(FILES ${CRUD_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/etc DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/bidder/data DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_subdirectory(jsonv)
add_subdirectory(CRUD)
add_subdirectory(rtb)
add_subdirectory(examples)
