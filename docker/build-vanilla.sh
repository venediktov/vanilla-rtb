#!/bin/bash

set -eu

[[ ! -d vanilla-rtb ]] && git clone --recursive https://github.com/venediktov/vanilla-rtb.git

[[ ! -d ../build/vanilla-rtb ]] && mkdir -p ../build/vanilla-rtb
[[ ! -d ../pkg/vanilla-rtb/snapshot ]] && mkdir -p ../pkg/vanilla-rtb/snapshot
cd ../build/vanilla-rtb

cmake -G 'Unix Makefiles'\
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$(readlink -f ../../pkg/vanilla-rtb/snapshot)" \
    "$(readlink -f ../../code/vanilla-rtb)"

make -j$(nproc) -l$(nproc) install

