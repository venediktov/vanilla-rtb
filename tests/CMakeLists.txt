FIND_PACKAGE(Threads REQUIRED)

SET(Boost_USE_MULTITHREADED ON)  
FIND_PACKAGE(Boost "${RTB_BOOST_VERSION}" REQUIRED COMPONENTS unit_test_framework REQUIRED)
ADD_DEFINITIONS("-DBOOST_TEST_DYN_LINK")

## Configuring vanilla-rtb
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")

ADD_EXECUTABLE(vanilla_tests
        vanilla_all.cpp
        vanilla_string_utils_test.cpp
        main.cpp)

TARGET_INCLUDE_DIRECTORIES(vanilla_tests SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(vanilla_tests ${Boost_LIBRARY_DIRS})
TARGET_LINK_LIBRARIES(vanilla_tests ${Boost_LIBRARIES})

ADD_CUSTOM_TARGET(unit-test vanilla_tests DEPENDS vanilla_tests USES_TERMINAL)

ADD_TEST(ctest_vanilla_tests "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target unit-test)
