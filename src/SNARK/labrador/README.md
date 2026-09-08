# LaBRADOR aggregation (via ICICLE)

`aggregate_labrador.cpp` aggregates *k* Generalized-XMSS multi-signatures into one
compact, post-quantum (Module-SIS) proof using **LaBRADOR** over Ingonyama's
**ICICLE** library — the chosen replacement for the FRI/hash route (Aurora/Ligero).

## Build & run

It builds against `ingonyama-zk/icicle-labrador`, which vendors ICICLE v4 (CPU or CUDA).

```bash
git clone https://github.com/ingonyama-zk/icicle-labrador.git
cd icicle-labrador

# One-line fix for GCC >= 13: give HOST_INLINE the `inline` keyword so
# __attribute__((always_inline)) does not fail with
#   "function body can be overwritten at link time".
sed -i 's/#define HOST_INLINE __attribute__((always_inline))/#define HOST_INLINE inline __attribute__((always_inline))/' \
    icicle/include/icicle/utils/modifiers.h

# Drop in the aggregation harness + register the target
cp /path/to/PQ-Etherium-implementation/src/SNARK/labrador/aggregate_labrador.cpp src/
#   then add an `aggregate_labrador` executable to src/CMakeLists.txt
#   (same source list as `example`, but aggregate_labrador.cpp instead of example.cpp)

bash run.sh                 # CPU  build + runs example
./build/src/aggregate_labrador CPU
# GPU (needs CUDA toolkit): bash run.sh -d CUDA ; ./build/src/aggregate_labrador CUDA
```

## What it does

1. Flattens the *k* signatures to a bit stream.
2. Packs the bits as a **low-norm** witness `S in Rq^{r*n}` (LaBRADOR needs a
   small-norm witness — the bit-decomposition of the signatures is exactly that).
3. Builds LaBRADOR EQ + ConstZero constraints satisfied by `S`, proves knowledge
   of the witness, and verifies.

## Reference-implementation verification harness (`aggregate_labrador_verify.c`)

`aggregate_labrador.cpp` (above) uses the icicle demo with a generic relation.
`aggregate_labrador_verify.c` instead uses the **Beullens–Seiler reference LaBRADOR**
(`github.com/lattice-dogs/labrador`) and encodes the **actual aggregate-verification
relation** via the Dachshund (linear-constraint) front end:

    for each signer i:  <A_i, sig_i> = root_i ,  sig_i short (ternary), norm-bounded

where `A_i` is the composed linear chain+Merkle verification map (its entries are the
`chain_step^(BASE-1-x_c)` and `node_mult^(HGT-1-l)` multipliers) and `root_i` is the
public tree root. This is LaBRADOR's native relation, so it needs no hash gadget.

Measured (k=4 signers, rank-512 witnesses, run under Intel SDE — see below):
- aggregate statement satisfied ("all 4 verifications hold")
- a built-in **tamper check** corrupts a signature and confirms the statement is rejected
- Dachshund → Labrador, all verify steps pass, **total proof 73.48 KB**

Build + run (requires AVX512; use Intel SDE on non-AVX512 CPUs):

```bash
git clone https://github.com/lattice-dogs/labrador.git
cd labrador
sed -i 's/-march=native -mtune=native/-march=icelake-server -mtune=icelake-server/' Makefile
cp /path/to/PQ-Etherium-implementation/src/SNARK/labrador/aggregate_labrador_verify.c .
SRC="pack.c greyhound.c dachshund.c chihuahua.c labrador.c data.c jlproj.c polx.c poly.c \
     polz.c sparsemat.c ntt.S invntt.S aesctr.c fips202.c randombytes.c cpucycles.c"
gcc -std=c2x -fwrapv -march=icelake-server -O3 aggregate_labrador_verify.c $SRC -o aggregate_labrador_verify -lm
# Intel SDE (download+extract from downloadmirror.intel.com, no root):
sde64 -icx -- ./aggregate_labrador_verify
```

## Scope / honesty

- `aggregate_labrador_verify.c` proves the **real aggregate-verification relation**, but
  with a **linear** model hash (chosen so XMSS verification fits LaBRADOR's linear
  constraints and is provable today). A linear hash is not collision-resistant; a real
  SHA-256/Poseidon means arithmetizing the hash into LaBRADOR's *quadratic* constraints —
  the remaining research work. The relation *form* (k short-preimage/opening constraints
  binding public roots to secret signatures, with a norm bound) is correct.
- `aggregate_labrador.cpp` (icicle) binds to the real signature witness but uses the
  system's generic relation. See `../README_PROOFSYSTEMS.md`.
