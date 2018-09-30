FROM vanillartb/vanilla-runtime:0.0.2
ARG BOOST_VERSION=1.67.0
ARG VANILLA_RTB_VERSION=snapshot
ARG WORK_ROOT=/root
LABEL Description="vanilla-rtb Dev" Vendor="ForkBid" Maintainer="mrbald@github"
RUN apt-get update
RUN apt-get install -yq --no-install-suggests --no-install-recommends make cmake git build-essential g++-7 vim python
ENV CC=gcc-7 CXX=g++-7

WORKDIR ${WORK_ROOT}/scripts
ADD build-vanilla.sh ${WORK_ROOT}/scripts
RUN chmod +x ${WORK_ROOT}/scripts/build-vanilla.sh && sync

WORKDIR ${WORK_ROOT}/deps
WORKDIR ${WORK_ROOT}/build
RUN env VANILLA_RTB_VERSION=${VANILLA_RTB_VERSION} PKG_DIR=${WORK_ROOT}/pkg DEPS_DIR=${WORK_ROOT}/deps BUILD_DIR=${WORK_ROOT}/build BOOST_VERSION=${BOOST_VERSION} ${WORK_ROOT}/scripts/build-vanilla.sh

WORKDIR /root/pkg/vanilla-rtb-${VANILLA_RTB_VERSION}/bin

CMD ["bash"]
