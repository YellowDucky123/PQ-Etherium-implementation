#!/bin/bash
# Build + run the Aurora (libiop) multi-signature aggregation demo.
#
# Prereq: a built copy of libiop (https://github.com/scipr-lab/libiop).
#   git clone --recursive https://github.com/scipr-lab/libiop.git
#   cd libiop
#   # one-line fix for modern GCC (>=13): add <cstddef> to libiop/algebra/utils.hpp
#   sed -i '0,/#include <cstdint>/s//#include <cstddef>\n#include <cstdint>/' libiop/algebra/utils.hpp
#   mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j iop
#
# Then point LIBIOP at that checkout and run this script.
set -e
LIBIOP="${LIBIOP:-$HOME/libiop}"
HERE="$(cd "$(dirname "$0")" && pwd)"

g++ -std=c++17 -O2 "$HERE/aggregate_aurora.cpp" \
  -I"$LIBIOP" -I"$LIBIOP/depends/libff" -I"$LIBIOP/depends/libfqfft" \
  -I/usr/include/x86_64-linux-gnu \
  "$LIBIOP/build/libiop/libiop.a" \
  "$LIBIOP/build/depends/libff/libff/libff.a" \
  "$LIBIOP/build/depends/libzm.a" \
  -lgmp -lsodium -lcrypto -fopenmp \
  -o "$HERE/aggregate_aurora"

echo "built: $HERE/aggregate_aurora"
"$HERE/aggregate_aurora"
