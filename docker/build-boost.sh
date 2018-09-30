#!/bin/bash

set -eu

if [[ ${BOOST_VERSION} != 'default' ]]; then
    readonly BOOST_URL="https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION//\./_}.tar.gz"
    readonly BOOST_DEPS_DIR="${DEPS_DIR}/boost-${BOOST_VERSION}"
    readonly BOOST_BUILD_DIR="${BUILD_DIR}/boost-${BOOST_VERSION}"
    readonly BOOST_TOOLSET=gcc
    readonly BOOST_LIBS='program_options,system,regex,serialization,log,date_time,test,filesystem'

    $(type -P rm) -rf "${BOOST_BUILD_DIR}"
    $(type -P mkdir) -p "${BOOST_BUILD_DIR}"

    $(type -P wget) --no-check-certificate --quiet -O - "${BOOST_URL}" | $(type -P tar) --strip-components=1 -xz -C "${BOOST_BUILD_DIR}"

    cd "${BOOST_BUILD_DIR}"
    ./bootstrap.sh --with-libraries="${BOOST_LIBS}" --with-toolset="${BOOST_TOOLSET}" || { cat bootstrap.log; exit 1; }
    ./b2 -d0 -q toolset="${BOOST_TOOLSET}" variant=release link=shared threading=multi runtime-link=shared install --prefix="${BOOST_DEPS_DIR}"

    $(type -P rm) -rf "${BOOST_BUILD_DIR}"
    $(type -P rmdir) --ignore-fail-on-non-empty "${BUILD_DIR}"

    export CMAKE_OPTIONS+=" -DBOOST_ROOT='${BOOST_DEPS_DIR}'"

    echo "Boost v${BOOST_VERSION} installed to ${BOOST_DEPS_DIR}"
else
    echo "Nothing done - using the default Boost installation"
fi
