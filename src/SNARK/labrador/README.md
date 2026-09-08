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

## Scope / honesty

The proof binds to the real signature witness and exercises the full LaBRADOR
prove/verify pipeline. The constraints are LaBRADOR's native committed
inner-product relation; full arithmetization of XMSS verification (hash chains +
Merkle path) into that relation is the remaining research work. See
`../README_PROOFSYSTEMS.md`.
