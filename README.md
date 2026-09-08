# PQ-Etherium Proof of Concept 
This repository is a proof of concept for the PQ-Etherium Project with Hash-Based Multi-Signatures based on the paper 
```
 @misc{cryptoeprint:2025/055,
      author = {Justin Drake and Dmitry Khovratovich and Mikhail Kudinov and Benedikt Wagner},
      title = {Hash-Based Multi-Signatures for Post-Quantum Ethereum},
      howpublished = {Cryptology {ePrint} Archive, Paper 2025/055},
      year = {2025},
      doi = {10.62056/aey7qjp10},
      url = {https://eprint.iacr.org/2025/055}
}
```

Update or fixes getting worked on in the `update` branch

The Signature Scheme now **compiles and passes a `sign → verify` round-trip** (with negative tests:
wrong message / tampered signature / wrong epoch are all rejected) — see
[`src/signature/tests/test_roundtrip.cpp`](src/signature/tests/test_roundtrip.cpp) and `test.cpp`.
It is self-consistent; it has **not** yet been checked byte-for-byte against the reference `hash-sig`
(matching test vectors is the remaining validation). The PQ-SNARK aggregation, originally
only "semi-finished", now has **two working transparent post-quantum proof backends** — Aurora
(hash/FRI, via `libiop`) and LaBRADOR (lattice / Module-SIS, via Ingonyama's `ICICLE`) — both
building and verifying end-to-end, plus a **real R1CS arithmetization of signature verification**
under Aurora. See [`src/SNARK/README_PROOFSYSTEMS.md`](src/SNARK/README_PROOFSYSTEMS.md) for the
full write-up, measured results, build steps, and the remaining work.

Some OOP design paradigms might need to be changed for the Multi-Signature aggregation to Work, for reference, look at `generalized_xmss.hpp` and notice that to have a group of `vector<PublicKey>`, it proves to be quite hard with the current design as the MessageHash template argument has integers which aren't a finite field. 

## Proposed changes
Use "true" OOP design paradigm and use object instantiations for most of it. Limit type Static functions. This was done for half of the project (e.g. `ShaTweakHash` is under this design). 
However, things like the basic_winternitz was changed to the more static approach, however, making these changes back and forth prove to be fairly trivial. But was not done because of deadline

### Effects
If this proposed changes were to be done, the multi signature aggregation algorithm might prove to be relatively easy. 
1. Move all number template arguments as class data.
2. ride the wave of effects from (1), e.g. make static functions non-static and use object instantiations.

However, it is also possible that by doing this, it might slow down the Scheme as more memory allocation and retrieval is required for the objects. 

# PQ-SNARK status (update branch)
The SNARK aggregation now builds and verifies with two backends — see
[`src/SNARK/README_PROOFSYSTEMS.md`](src/SNARK/README_PROOFSYSTEMS.md). What is done vs. remaining:

**Done**
- **Aurora** (`libiop`) and **LaBRADOR** (`ICICLE`) both build and run end-to-end (CPU),
  each proving+verifying an aggregation instance bound to real (pk, sig) data.
- A **real R1CS verification circuit** (`src/SNARK/aurora/aggregate_aurora_verify.cpp`)
  arithmetizes `verify()` in three stages — encode → Winternitz chains → Merkle path — as
  genuine enforced constraints (with a tamper check proving they bite).

- A **real LaBRADOR aggregate-verification relation** (`src/SNARK/labrador/aggregate_labrador_verify.c`,
  Beullens–Seiler reference impl via the Dachshund front end): for each signer `<A_i, sig_i> = root_i`
  with a short norm-bounded signature and `A_i` the composed linear chain+Merkle map (also tamper-checked).
- The fixed core **plugs into the SNARK**: `src/SNARK/aurora/bridge_core_to_aurora.cpp` generates real
  signatures with `src/signature/`, verifies them natively, and drives a real Aurora aggregation proof
  (k=4 → ~131 KB, VERIFICATION SUCCESS).

**Remaining**
- Both verification relations use a **model** hash — a MiMC cubing sponge (Aurora R1CS) and a *linear*
  map (LaBRADOR/Dachshund) — not SHA-256/Poseidon. A real hash gadget (R1CS for Aurora; LaBRADOR's
  *quadratic* constraints for the lattice side) is the main outstanding piece.
- The original OOP blocker for grouping `vector<PublicKey>`/`vector<Signature>` remains for a
  fully generic multi-signature API.

## Results (measured on this machine)
CPU: Intel i7-10750H (AVX2, **no AVX512**), 8 GB RAM, GCC 13.3, no CUDA toolkit.
Full write-up and build steps in [`src/SNARK/README_PROOFSYSTEMS.md`](src/SNARK/README_PROOFSYSTEMS.md).

| Backend / program | What it proves | Proof size | Prove / Verify | Verified |
|---|---|---|---|---|
| **Aurora** `aggregate_aurora_verify.cpp` | **Real** R1CS: encode → Winternitz chains → Merkle path (k=4), model MiMC hash | ~131 KB | 0.34 s / 0.03 s | ✓ (+ tamper rejected) |
| Aurora `aggregate_aurora.cpp` | data-bound scaffold, generic R1CS (k=8) | ~210 KB | 6.6 s / 0.25 s | ✓ |
| **LaBRADOR (reference)** `aggregate_labrador_verify.c` | **Real** aggregate verification `⟨A_i,sig_i⟩=root_i` (k=4), Dachshund→Labrador, linear hash | **73.5 KB** | — (under SDE) | ✓ (+ tamper rejected) |
| LaBRADOR (icicle demo) `aggregate_labrador.cpp` | generic relation on real signature witness (k=8) | ~1.5 MB (demo floor) | 50 s / 40 s | ✓ |
| LaBRADOR ref. Chihuahua (constraint-system) | 2 dot-product constraints, rank 2¹¹ | 34.7 KB pack | 3.3 s / 1.8 s (SDE) | ✓ |
| LaBRADOR ref. Greyhound (poly commitment, aside) | eval proof over a 2²⁶ polynomial | 50.6 KB pack | 68 s / 28 s (SDE) | ✓ |

Notes: LaBRADOR reference-impl figures were run under **Intel SDE** emulation (this CPU lacks AVX512);
SDE is ~50× slower than native AVX512 would be. The ~50 KB LaBRADOR headline comes from the AVX512
reference impl, **not** the readable icicle demo (whose recursion floors at ~1.3–1.7 MB).

## Possibly some bugs and syntax errors
(Several core-compile blockers were fixed on the `update` branch — e.g. the angle-bracket
`endian.hpp` includes in `src/symmetric/tweak_hash/{sha,blake}.hpp`.)

# What is in the repository 
1. A Generalized XMSS implementation (compiles + sign/verify round-trip tested; see `src/signature/tests/test_roundtrip.cpp`)
2. A Basic Winternitz OTS Scheme
3. A Target-Sum Winternitz OTS Scheme
4. SHA messageHash, TweakHash, and PRF
5. Blake3 messageHash and TweakHash
6. An probable implementation of a multiSignature dataStructure/class
7. A working multi-signature aggregation SNARK with **two backends** — Aurora (`libiop`) and
   LaBRADOR (`ICICLE`) — plus a real R1CS arithmetization of signature verification under Aurora.
   See [`src/SNARK/README_PROOFSYSTEMS.md`](src/SNARK/README_PROOFSYSTEMS.md).
8. A "Finished" Python version of the Signature Scheme. ( The XMSS is **allegedly** Finished and tested in the Python version, Testing was not official )

# Building
Preffered version = C++ V23, -std=c++23
No concrete Build instructions have been made, However, here are the information needed to Build. 
1. Most Implementations (almost all) are in the headers, Therefore, to use, simply put the highest ranking header to use in a .cpp or .tcc, etc.
For example, to use the Winternitz OTS scheme, simply put ```#include "src/inc_encoding/basic_winternitz.hpp"``` in a .cpp or equivalent file.

2. For the SNARK aggregation, see [`src/SNARK/README_PROOFSYSTEMS.md`](src/SNARK/README_PROOFSYSTEMS.md).
   - Aurora: build `libiop` (add `#include <cstddef>` to `libiop/algebra/utils.hpp` for GCC ≥ 13),
     then run `src/SNARK/aurora/build.sh` (set `LIBIOP=/path/to/libiop`). Builds and runs both the
     data-bound scaffold and the real verification circuit.
   - LaBRADOR: clone `ingonyama-zk/icicle-labrador` (one-line `HOST_INLINE` fix for GCC ≥ 13),
     drop in `src/SNARK/labrador/aggregate_labrador.cpp`; see `src/SNARK/labrador/README.md`.

# Files
File names are self explanatory

## Incomparable Encoding
- `src/inc_encoding/basic_winternitz.hpp`: A Basic Winternitz OTS
- `src/inc_encoding/target_sum.hpp`: A Target-Sum Winternitz OTS
- `src/symmetric/inc_encoding.hpp`: an abstract class for Incomparable Encoding

## XMSS
- `src/signature/generalized_xmss.hpp`: A General XMSS
- `src/symmetric/tweak_hash_tree.hpp`: Functions supporting the General XMSS

## SNARK
See [`src/SNARK/README_PROOFSYSTEMS.md`](src/SNARK/README_PROOFSYSTEMS.md) for the overview.
- `src/SNARK/aurora/aggregate_aurora_verify.cpp`: the **real** 3-stage verification R1CS
  (encode → Winternitz chains → Merkle path), proven + verified with Aurora
- `src/SNARK/aurora/aggregate_aurora.cpp`: data-bound Aurora scaffold (+ `build.sh`)
- `src/SNARK/labrador/aggregate_labrador.cpp`: LaBRADOR aggregation via ICICLE (+ `README.md`)
- `src/SNARK/myR1CS.hpp`, `src/SNARK/aggregate.cpp`: **superseded** original stubs (did not
  compile / encoded no real relation), kept for reference

## Hashes
- `src/symmetric/message_hash.hpp`: An abstract class for message_hashes
- `src/symmetric/message_hash/sha.hpp`: A SHA256 instantiation for the message_hash
- `src/symmetric/message_hash/blake3.hpp`: A Blake3 instantiation for the message_hash
- `src/symmetric/TweakHash.hpp`: An abstract class for a Tweakable Hash function
- `src/symmetric/tweak_hash/sha.hpp`: A SHA256 Instantiation for a Tweakable Hash function
- `src/symmetric/tweak_hash/blake.hpp`: A Blake3 Instantiation for a Tweakable Hash function

## PRF
- `src/symmetric/prf.hpp`: An abstract class for pseudorandom functions
- `src/symmetric/prf/sha.hpp`: A SHA256 Pseudorandom Function instantiation

# Post Quantum Claim 
## Hash
On a not so analytical manner, This scheme is PQ because it uses Post Quantum secure hash Functions as it's underlying functionality. The SHA256 is one that has had many cryptoanalysis attacks done to, however, there are still no known attacks to SHA256 that are better than the Grover's attack at the time of this writing. The BLAKE3 is a newer hash function but also does not have known attacks better than Grover's attack.

## XMSS Many Time signature
The XMSS security claim is such that, it's underlying functionality is still an OTS. Furthermore, to limit it's data surface, this instantiation uses epochs and lifetimes which increase it's efficiency and security with "not so many" signatures under one public key.

## SNARK
Both aggregation backends are transparent (no trusted setup) and plausibly post-quantum:
**Aurora** rests on collision-resistant hashing (FRI/BCS), and **LaBRADOR** on the Module-SIS
lattice assumption. Note the current verification circuit uses a *model* hash for the
arithmetization, so its concrete security depends on later swapping in a real
SHA-256/Poseidon gadget — see [`src/SNARK/README_PROOFSYSTEMS.md`](src/SNARK/README_PROOFSYSTEMS.md).
