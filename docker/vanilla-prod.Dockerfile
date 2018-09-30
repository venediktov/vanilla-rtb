FROM vanillartb/vanilla-runtime:0.0.2
ARG VANILLA_RTB_VERSION=snapshot
ARG WORK_ROOT=/root
LABEL Description="VanillaRTB Prod" Vendor="ForkBid" Maintainer="mrbald@github"
ADD pkg ${WORK_ROOT}/pkg
WORKDIR ${WORK_ROOT}/pkg/vanilla-rtb-${VANILLA_RTB_VERSION}/bin
CMD ["bash"]
