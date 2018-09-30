FROM vanillartb/vanilla-base:0.0.2
ARG BOOST_VERSION=1.67.0
ARG WORK_ROOT=/root
LABEL Description="VanillaRTB Dependencies Builder" Vendor="ForkBid" Maintainer="mrbald@github"
RUN apt-get install -yq --no-install-suggests --no-install-recommends wget build-essential g++-7
ENV CC=gcc-7 CXX=g++-7

WORKDIR ${WORK_ROOT}/scripts
ADD build-boost.sh ${WORK_ROOT}/scripts
RUN chmod +x ${WORK_ROOT}/scripts/build-boost.sh && sync

WORKDIR ${WORK_ROOT}/deps
WORKDIR ${WORK_ROOT}/build
RUN env DEPS_DIR=${WORK_ROOT}/deps BUILD_DIR=${WORK_ROOT}/build BOOST_VERSION=${BOOST_VERSION} ${WORK_ROOT}/scripts/build-boost.sh

CMD ["bash"]
