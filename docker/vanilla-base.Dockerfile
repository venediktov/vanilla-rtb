FROM debian:stretch
LABEL Description="VanillaRTB Base" Vendor="ForkBid" Maintainer="mrbald@github"
RUN apt-get update && apt-get install -y apt-utils
RUN apt-get install -y\
 libboost-atomic1.62.0\
 libboost-chrono1.62.0\
 libboost-date-time1.62.0\
 libboost-filesystem1.62.0\
 libboost-log1.62.0\
 libboost-program-options1.62.0\
 libboost-regex1.62.0\
 libboost-serialization1.62.0\
 libboost-system1.62.0\
 libboost-thread1.62.0

CMD ["bash"]
