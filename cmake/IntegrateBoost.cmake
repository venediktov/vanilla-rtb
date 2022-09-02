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
  execute_process(COMMAND bash -c "cd ${BOOST_ROOT_DIR} && wget ${BOOST_URL}")
  execute_process(COMMAND bash -c "cd ${BOOST_ROOT_DIR} && tar -xvzf ./*.tar.gz")
  execute_process(COMMAND bash -c "cd ${BOOST_ROOT_DIR} && rm ./*.tar.gz")
  execute_process(COMMAND bash -c "cd ${BOOST_ROOT_DIR} && mv boost* boost")
endmacro()


macro(install_boost)
  message(STATUS "Installing boost...")
  execute_process(COMMAND mkdir -p ${BOOST_INSTALL_DIR})
  execute_process(
    COMMAND 
    bash -c "cd ${BOOST_SRC_DIR} && ./bootstrap.sh --prefix=${BOOST_INSTALL_DIR}")
  execute_process(COMMAND bash -c "cd ${BOOST_SRC_DIR} && ./b2 install")
endmacro()


macro(config_boost)
  message(STATUS "Config boost lib...")
  set(BOOST_ROOT ${BOOST_INSTALL_DIR})
  set(Boost_INCLUDE_DIR ${BOOST_INSTALL_DIR}/include)
  set(Boost_LIBRARY_DIRS ${BOOST_INSTALL_DIR}/lib)
  find_package(Boost 1.67.0 REQUIRED)
endmacro()


macro(integrate_boost)
  config_boost()
  if(Boost_FOUND)
    message(STATUS "Boost lib already exists")
  else()
    download_boost()
    install_boost()
    config_boost()
  endif()
endmacro()
