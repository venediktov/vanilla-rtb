# -*- cmake -*-

# the cmake version used in the travis integration build
# older versions may still work but have not been tested
cmake_minimum_required(VERSION 3.7.2)

project(vanilla-rtb CXX)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

FIND_PACKAGE(Boost 1.60.0 REQUIRED)

FIND_PACKAGE(Threads)

IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.1")
    MESSAGE(FATAL_ERROR "The build has not been tested with GCC version < 5.1, to proceed at your own risk comment this message out in the top level CMakeLists.txt")
ENDIF()

IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
ENDIF()

IF (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET ( CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/examples )
ENDIF()

message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_CURRENT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}")

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

IF (UNIX AND NOT APPLE)
    SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/../lib64")
ELSE (UNIX AND NOT APPLE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib64")
ENDIF (UNIX AND NOT APPLE)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#Install CRUD libraries
#file(GLOB CRUD_LIB ${CMAKE_CURRENT_BINARY_DIR}/CRUD/service/*.so)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/CRUD/service/libcrud_service.so DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)


#Install all json files from our examples
file(GLOB_RECURSE JSON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.json)
INSTALL(FILES ${JSON_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

#Install all shell scripts running our samples
file(GLOB_RECURSE SHELL_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.sh)
INSTALL(FILES ${SHELL_SCRIPTS}  
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/etc DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/bidder/data DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/UI/CampaignBudgetsCDN/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/www/campaign)



add_subdirectory(jsonv)
add_subdirectory(CRUD)
add_subdirectory(rtb)
add_subdirectory(parsers)
IF(NOT DEFINED VANILLA_NO_EXAMPLES)
add_subdirectory(examples)
ENDIF()
add_subdirectory(benchmarks)
