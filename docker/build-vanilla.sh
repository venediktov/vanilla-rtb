#!/bin/bash

set -eu

readonly VANILLA_RTB_PKG_DIR="${PKG_DIR}/vanilla-rtb-${VANILLA_RTB_VERSION}"
readonly VANILLA_RTB_BUILD_DIR="${BUILD_DIR}/vanilla-rtb-${VANILLA_RTB_VERSION}"

$(type -P rm) -rf ${VANILLA_RTB_BUILD_DIR} ${VANILLA_RTB_PKG_DIR}

git -c http.sslVerify=false clone --recursive https://github.com/venediktov/vanilla-rtb.git ${VANILLA_RTB_BUILD_DIR}

$(type -P mkdir) ${VANILLA_RTB_BUILD_DIR}/Release
cd ${VANILLA_RTB_BUILD_DIR}/Release

CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX='${VANILLA_RTB_PKG_DIR}'"
if [[ ${BOOST_VERSION} != 'default' ]]; then
    readonly BOOST_DEPS_DIR="${DEPS_DIR}/boost-${BOOST_VERSION}"
    export CMAKE_OPTIONS+=" -DBOOST_ROOT='${BOOST_DEPS_DIR}'"
fi

cmake .. -G 'Unix Makefiles' ${CMAKE_OPTIONS}
cmake --build . --target install -- -j$(nproc) -l$(nproc)

cd ${BUILD_DIR}
$(type -P rm) -rf ${VANILLA_RTB_BUILD_DIR}

