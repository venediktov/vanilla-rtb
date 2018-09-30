FROM vanillartb/vanilla-base:0.0.2
LABEL Description="VanillaRTB Prod" Vendor="ForkBid" Maintainer="mrbald@github"
WORKDIR /root/pkg/vanilla
ADD vanilla .
CMD ["bash"]
