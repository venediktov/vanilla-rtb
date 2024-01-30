# file: IntegrateBoost.cmake
# date: 2022-09-02


set(
  BOOST_URL 
  "https://boostorg.jfrog.io/artifactory/main/release/1.67.0/source/boost_1_67_0.tar.gz"
)

set(BOOST_ROOT_DIR ${CMAKE_SOURCE_DIR}/build/_boost)
set(BOOST_SRC_DIR ${BOOST_ROOT_DIR}/boost)
set(BOOST_INSTALL_DIR ${BOOST_ROOT_DIR}/installation)


macro(download_boost)
  message(STATUS "Downloading boost...")
  message(STATUS "Target boost URL: ${BOOST_URL}")
  execute_process(COMMAND mkdir -p ${BOOST_ROOT_DIR})
  if (NOT EXISTS ${BOOST_SRC_DIR})
     file(DOWNLOAD ${BOOST_URL} ${BOOST_ROOT_DIR}/boost_1_67_0.tar.gz SHOW_PROGRESS)
     execute_process(COMMAND bash -c "cd ${BOOST_ROOT_DIR} && tar -xvzf ./*.tar.gz")
     execute_process(COMMAND bash -c "cd ${BOOST_ROOT_DIR} && rm ./*.tar.gz")
     execute_process(COMMAND bash -c "cd ${BOOST_ROOT_DIR} && mv boost* boost")
  endif()
endmacro()


macro(install_boost)
  message(STATUS "Installing boost...")
  execute_process(COMMAND mkdir -p ${BOOST_INSTALL_DIR})
  execute_process(
    COMMAND 
    bash -c "cd ${BOOST_SRC_DIR} && ./bootstrap.sh --prefix=${BOOST_INSTALL_DIR} cxxflags=-std=c++17")
  execute_process(COMMAND bash -c "cd ${BOOST_SRC_DIR} && ./b2 -j4 hardcode-dll-paths=true dll-path=\"'\\$ORIGIN/../lib'\" install")
endmacro()


macro(config_boost)
  message(STATUS "Config boost lib...")
  set(BOOST_ROOT ${BOOST_INSTALL_DIR})
  set(Boost_INCLUDE_DIR ${BOOST_INSTALL_DIR}/include)
  set(Boost_LIBRARY_DIRS ${BOOST_INSTALL_DIR}/lib)
  find_package(Boost 1.67.0 EXACT REQUIRED)
endmacro()


macro(integrate_boost)
  if(Boost_FOUND)
    message(STATUS "Boost lib already exists")
  else()
    download_boost()
    install_boost()
    config_boost()
  endif()
endmacro()
