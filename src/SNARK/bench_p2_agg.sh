#!/bin/bash
# Benchmark: aggregate k REAL Poseidon2-BN254 Generalized-XMSS signatures (C++ core)
# with Aurora (libiop, R1CS) and Plonky3 (uni-stark, AIR). Both prove the same relation
# from the same instance file.
#
#   LIBIOP=~/test/SNARK/libiop ./bench_p2_agg.sh [k ...]        (default k: 4 8 16 32)
#
# Env: AURORA_ZK="1 0" (which zk modes to run), P3_FRI="56:2" (queries:log_blowup pairs),
#      OUT=<dir> (default ./bench_out). Writes $OUT/results.tsv and per-run logs.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
LIBIOP="${LIBIOP:-$HOME/libiop}"
OUT="${OUT:-$HERE/bench_out}"
KS="${*:-4 8 16 32}"
AURORA_ZK="${AURORA_ZK:-1 0}"
P3_FRI="${P3_FRI:-56:2}"
mkdir -p "$OUT"

FFI="$HERE/plonky3_ffi"
cargo build --release --manifest-path "$FFI/Cargo.toml"
cargo build --release --manifest-path "$HERE/plonky3_air/Cargo.toml"
P3="$HERE/plonky3_air/target/release/p3_xmss_agg"

g++ -std=c++23 -O2 -I"$ROOT" "$HERE/export_p2_sigs.cpp" "$ROOT/src/symmetric/prf/sha.cpp" \
    "$FFI/target/release/libp3_poseidon2_ffi.a" -lcrypto -lpthread -ldl -lm -o "$OUT/export_p2_sigs"

INCS="-I$LIBIOP -I$LIBIOP/depends/libff -I$LIBIOP/depends/libfqfft -I/usr/include/x86_64-linux-gnu -I$FFI"
LIBS="$LIBIOP/build/libiop/libiop.a $LIBIOP/build/depends/libff/libff/libff.a $LIBIOP/build/depends/libzm.a -lgmp -lsodium -lcrypto -fopenmp"
g++ -std=c++17 -O2 "$HERE/aurora/aggregate_aurora_verify.cpp" $INCS $LIBS \
    "$FFI/target/release/libp3_poseidon2_ffi.a" -lpthread -ldl -lm -o "$OUT/aggregate_aurora_verify"

TSV="$OUT/results.tsv"
echo -e "system\tk\tvariant\tsize_param\tproof_bytes\tprove_ms\tverify_ms\tpeak_rss_mb\tok" > "$TSV"

# run <log> <cmd...>: runs under GNU time, prints the RESULT line + peak RSS (MiB)
run() {
    local log="$1"; shift
    /usr/bin/time -v "$@" > "$log" 2> "$log.time" || { echo "FAILED: $* (see $log)"; return 1; }
    local rss_kb; rss_kb=$(awk -F: '/Maximum resident/ {gsub(/ /,"",$2); print $2}' "$log.time")
    echo "$(grep '^RESULT' "$log") rss_mb=$((rss_kb / 1024))"
}
field() { echo "$1" | tr ' ' '\n' | awk -F= -v k="$2" '$1==k {print $2}'; }

for k in $KS; do
    inst="$OUT/sigs_k$k.txt"
    "$OUT/export_p2_sigs" "$k" 5 "$inst"
    for zk in $AURORA_ZK; do
        r=$(run "$OUT/aurora_k${k}_zk$zk.log" "$OUT/aggregate_aurora_verify" "$inst" "$zk")
        echo "$r"
        echo -e "aurora\t$k\tzk=$zk\tconstraints=$(field "$r" padded)\t$(field "$r" proof_bytes)\t$(field "$r" prove_ms)\t$(field "$r" verify_ms)\t$(field "$r" rss_mb)\t$(field "$r" ok)" >> "$TSV"
    done
    for qb in $P3_FRI; do
        q="${qb%%:*}"; b="${qb##*:}"
        r=$(run "$OUT/plonky3_k${k}_q${q}_b$b.log" "$P3" "$inst" "$q" "$b")
        echo "$r"
        echo -e "plonky3\t$k\tq=$q,log_blowup=$b\trows=$(field "$r" rows)\t$(field "$r" proof_bytes)\t$(field "$r" prove_ms)\t$(field "$r" verify_ms)\t$(field "$r" rss_mb)\t$(field "$r" ok)" >> "$TSV"
    done
done
echo; column -t -s $'\t' "$TSV"
