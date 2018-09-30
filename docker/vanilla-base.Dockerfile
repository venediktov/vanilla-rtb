FROM ubuntu:latest
LABEL Description="VanillaRTB Base" Vendor="ForkBid" Maintainer="mrbald@github"
RUN apt-get update && apt-get -yq --no-install-suggests --no-install-recommends install -y apt-utils
CMD ["bash"]
