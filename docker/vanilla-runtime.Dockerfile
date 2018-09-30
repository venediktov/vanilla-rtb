FROM vanillartb/vanilla-base:0.0.2
ARG BOOST_VERSION=1.67.0
ARG WORK_ROOT=/root
LABEL Description="VanillaRTB Runtime" Vendor="ForkBid" Maintainer="mrbald@github"
ADD deps ${WORK_ROOT}/deps
CMD ["bash"]
