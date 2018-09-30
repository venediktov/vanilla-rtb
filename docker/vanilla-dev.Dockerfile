FROM vanillartb/vanilla-base:0.0.2
LABEL Description="vanilla-rtb Dev" Vendor="ForkBid" Maintainer="mrbald@github"
RUN apt-get update
RUN apt-get install -y\
 cmake\
 git\
 clang\
 vim\
 libboost-atomic1.67-dev\
 libboost-chrono1.67-dev\
 libboost-date-time1.67-dev\
 libboost-filesystem1.67-dev\
 libboost-log1.67-dev\
 libboost-program-options1.67-dev\
 libboost-regex1.67-dev\
 libboost-serialization1.67-dev\
 libboost-system1.67-dev\
 libboost-thread1.67-dev\
 libboost-test1.67\
 libboost-test1.67-dev\
 python

WORKDIR /root/pkg
WORKDIR /root/build
WORKDIR /root/code

ADD build-vanilla.sh /root/code

RUN chmod +x ./build-vanilla.sh && sync && ./build-vanilla.sh

WORKDIR /root/pkg/vanilla-rtb/snapshot/bin

CMD ["bash"]
