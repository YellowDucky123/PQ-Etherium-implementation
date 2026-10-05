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

INCS="-I$LIBIOP -I$LIBIOP/depends/libff -I$LIBIOP/depends/libfqfft -I/usr/include/x86_64-linux-gnu"
LIBS="$LIBIOP/build/libiop/libiop.a $LIBIOP/build/depends/libff/libff/libff.a $LIBIOP/build/depends/libzm.a -lgmp -lsodium -lcrypto -fopenmp"

# Plonky3 Poseidon2-BN254 over FFI (Rust staticlib), used by the verification circuit (2).
FFI="$HERE/../plonky3_ffi"
cargo build --release --manifest-path "$FFI/Cargo.toml"
FFI_INC="-I$FFI"
FFI_LIB="$FFI/target/release/libp3_poseidon2_ffi.a -lpthread -ldl -lm"

# 1) generic data-bound scaffold (exercises the prover/verifier)
g++ -std=c++17 -O2 "$HERE/aggregate_aurora.cpp" $INCS $LIBS -o "$HERE/aggregate_aurora"
echo "built: $HERE/aggregate_aurora"; "$HERE/aggregate_aurora"

echo; echo "==================================================================="; echo
# 2) REAL 3-stage verification circuit (encode + Winternitz chains + Merkle path),
#    Poseidon2-BN254 hash, native side = Plonky3 via FFI
#    The instance is k=4 REAL signatures from the C++ core (Poseidon2TweakHash),
#    written by ../export_p2_sigs.
ROOT="$(cd "$HERE/../../.." && pwd)"
g++ -std=c++23 -O2 -I"$ROOT" "$HERE/../export_p2_sigs.cpp" "$ROOT/src/symmetric/prf/sha.cpp" \
    $FFI_LIB -lcrypto -o "$HERE/export_p2_sigs"
"$HERE/export_p2_sigs" 4 5 "$HERE/sigs_k4.txt"
g++ -std=c++17 -O2 "$HERE/aggregate_aurora_verify.cpp" $INCS $FFI_INC $LIBS $FFI_LIB -o "$HERE/aggregate_aurora_verify"
echo "built: $HERE/aggregate_aurora_verify"; "$HERE/aggregate_aurora_verify" "$HERE/sigs_k4.txt"

echo; echo "==================================================================="; echo
# 3) bridge: REAL Generalized-XMSS signatures (core) -> Aurora aggregation.
#    Core needs C++23 (concepts/byteswap); libiop needs C++17 -> split TUs.
g++ -std=c++23 -O2 -I"$ROOT" -c "$HERE/core_sigs.cpp" "$ROOT/src/symmetric/prf/sha.cpp"
g++ -std=c++17 -O2 -I"$HERE" $INCS "$HERE/bridge_core_to_aurora.cpp" core_sigs.o sha.o $LIBS -lcrypto \
    -o "$HERE/bridge_core_to_aurora"
rm -f core_sigs.o sha.o
echo "built: $HERE/bridge_core_to_aurora"; "$HERE/bridge_core_to_aurora"
