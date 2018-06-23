FROM vanillartb/vanilla-base:0.0.1
LABEL Description="vanilla-rtb Dev" Vendor="ForkBid" Maintainer="mrbald@github"
RUN apt-get update
RUN apt-get install -y\
 cmake\
 git\
 'g++'\
 vim\
 libboost-atomic1.62-dev\
 libboost-chrono1.62-dev\
 libboost-date-time1.62-dev\
 libboost-filesystem1.62-dev\
 libboost-log1.62-dev\
 libboost-program-options1.62-dev\
 libboost-regex1.62-dev\
 libboost-serialization1.62-dev\
 libboost-system1.62-dev\
 libboost-thread1.62-dev\
 libboost-test1.62.0\
 libboost-test1.62-dev\
 python

WORKDIR /root/pkg
WORKDIR /root/build
WORKDIR /root/code

ADD build-vanilla.sh /root/code

RUN  chmod +x ./build-vanilla.sh ; sync ;  ./build-vanilla.sh

WORKDIR /root/pkg/vanilla-rtb/snapshot/bin

CMD ["bash"]
