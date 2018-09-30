FROM debian:unstable
LABEL Description="VanillaRTB Base" Vendor="ForkBid" Maintainer="mrbald@github"
RUN apt-get update && apt-get install -y apt-utils
RUN apt-get install -yq\
 libboost-atomic1.67\
 libboost-chrono1.67\
 libboost-date-time1.67\
 libboost-filesystem1.67\
 libboost-log1.67\
 libboost-program-options1.67\
 libboost-regex1.67\
 libboost-serialization1.67\
 libboost-system1.67\
 libboost-thread1.67

CMD ["bash"]
