# Multi-signature aggregation: proof systems

The goal of `src/SNARK/` is to aggregate *k* Generalized-XMSS multi-signatures into
**one** succinct proof "these k signatures all verify for message m at epoch e."
Three transparent, post-quantum proof backends are now wired up and **all build and
verify end-to-end** on CPU (tested on this machine, GCC 13.3, no CUDA toolkit):

| Backend      | Kind                         | Dir                 | Status |
|--------------|------------------------------|---------------------|--------|
| **Aurora**   | hash/FRI zk-STARK (libiop)   | `aurora/`           | ✅ builds, proves, verifies |
| **LaBRADOR** | lattice / Module-SIS (ICICLE)| `labrador/`         | ✅ builds, proves, verifies |
| **Plonky3**  | uni-stark AIR over BN254     | `plonky3_air/`      | ✅ builds, proves, verifies (real Poseidon2 sigs) |

The original `myR1CS.hpp`, `myR1CS.tcc`, and `aggregate.cpp` are left in place for
reference but are **superseded** — they did not compile and did not encode a real
proof (see "What was wrong with the original" below).

## Measured results (CPU, k = 8 signers, ~2 KB signatures each)

| Backend  | Problem size                 | Prove   | Verify  | Proof size (total) |
|----------|------------------------------|---------|---------|--------------------|
| Aurora   | R1CS 32768 constr / 32767 var| 6.6 s   | 0.25 s  | ~210 KB            |
| LaBRADOR | Rq witness r=16, n=256, d=64 | ~50 s   | ~40 s   | ~1.5 MB            |

Notes:
- These are *this-machine CPU* numbers at demo parameters, not optimized/GPU figures.
- Aurora proof size grows with the circuit.
- **On the LaBRADOR proof size (important):** the LaBRADOR *paper* headline is ~50 KB,
  but the `icicle-labrador` **demo does not reach that**. Its recursion (`compute_mu_nu`,
  balanced to `r'^2 = n'/4`) converges to a fixed-point instance (~n=364, r=14) and the
  base-case proof floors at ~1.3–1.7 MB — adding recursion iterations past that point does
  not shrink it (measured trajectory below). Even the demo's own stock `example`/benchmark
  emits a ~2 MB proof. The floor is set by the babykoala parameter regime:
  `kappa = MSIS rank = 37`, ring degree `d = 64`, `|Zq| = 8 B`, times ~3x base
  decomposition — i.e. `t = r·kappa` dominates. Reaching the ~50 KB headline needs the
  tuned/production parameters from the paper, not this compact readable demo. The harness
  reports the **total** proof (per-recursion transcripts + final base case); pass a second
  CLI arg to set NUM_REC, e.g. `./aggregate_labrador CPU 6`.

  Measured recursion trajectory (k=8, r=16, n=256), `base-case proof size` per iteration:
  4.55 MB → 2.73 MB → 1.89 MB → 1.69 MB → 1.68 MB (fixed point) → 1.30 MB.

- **Compact proofs ARE achievable — with the reference implementation (verified here).** The
  Beullens–Seiler reference LaBRADOR (`github.com/lattice-dogs/labrador`, C) produces them. It
  requires **AVX512**, which this CPU (i7-10750H) lacks, so it was run under **Intel SDE**
  emulation (built with `-march=icelake-server`, run via `sde64 -icx -- ./test_<name>`; SDE
  downloaded from downloadmirror.intel.com, extracted, no sudo).

  The relevant target for *aggregation* is the **constraint-system** front-end, NOT Greyhound:
  - **Chihuahua** = LaBRADOR's principal statement: sparse **dot-product constraints over R_q +
    norm bound** (`init_sparsecnst_raw` / `set_sparsecnst_raw`). This is what we'd express
    signature verification in.
  - **Dachshund** = a simple statement: witness vectors with norm bounds + k **linear
    constraints**, reduced to LaBRADOR.
  - **Greyhound** = a *polynomial-commitment* application on top of LaBRADOR — not needed here.

  `test_chihuahua` under SDE (witness rank 2^11, 2 dot-product constraints; verification passed
  at every step, exit 0):
  - Chihuahua → LaBRADOR, two-layer: **total proof 36.13 KB**
  - Chihuahua Pack (composite): **34.68 KB**, **prove 3.29 s / verify 1.83 s even emulated**
  - witness shrinks through the recursion 2048 → 410/300 → 178/162 → …

  (Greyhound was also run once, giving a 50.56 KB pack for a 2^26-poly commitment, but it's the
  wrong tool for us and its 2^26 workload took ~8 min under emulation.)

  Takeaway: LaBRADOR's ~tens-of-KB proofs are real and come from the AVX512 reference impl (or
  production ICICLE), not the readable icicle-labrador demo. On non-AVX512 hardware they run only
  under emulation (SDE) — correct but ~50x slower; the small Chihuahua constraint statement is
  still only seconds emulated.

- **Can the icicle-labrador demo reach ~50 KB? No (tested).** The only in-repo knob for the fixed
  point is `C` in `compute_mu_nu` (`icicle/../src/shared.cpp`, default `1/4`). Lowering
  it does shrink the base case (C=1/16 → ~1.23 MB, C=1/64 → ~1.02 MB) but it **breaks
  verification**: the reduced-instance witness norm exceeds √q and the verifier aborts
  at `verifier.cpp:428` (`Input value … greater than sqrt(q)`, `INVALID_ARGUMENT`). So
  the smallest *verifying* proof here is ~1.3–1.7 MB. Reaching the paper's ~50 KB would
  need things this readable demo does not ship: a smaller MSIS rank (weakens security),
  a ring with larger degree `d` (only `babykoala`, d=64, is included), or a compact
  final base-case proof (e.g. a Greyhound-style terminator) plus the paper's tuned
  parameter set — i.e. production ICICLE LaBRADOR, not this demo.

## Aurora (libiop) — `aurora/`

Three programs built and run by `aurora/build.sh` (needs a built libiop — the script documents
the one-line `<cstddef>` fix for GCC ≥ 13 — plus `cargo` for the Plonky3 FFI crate). Programs 1 and 3
run over `libff::gf64`; program 2 runs over the BN254 scalar field (`alt_bn128_Fr`):

**1. `aggregate_aurora.cpp` — data-bound scaffold.** Maps the flattened statement
(k, epoch, message, k public keys) to the R1CS *primary* input and the flattened k
signatures to the *auxiliary* input, builds a satisfiable R1CS bound to that data, and
runs `aurora_snark_prover` / `aurora_snark_verifier`. The constraints are libiop's
generic multiplicative gadget — it exercises the pipeline but does not encode verification.

**2. `aggregate_aurora_verify.cpp` — the REAL verification circuit, Poseidon2 hash.**
Arithmetizes the relation from `generalized_xmss.hpp::verify()` as enforced R1CS constraints
over BN254 Fr, with every hash = Poseidon2-BN254 (t=3, x^5, R_F=8, R_P=56, HorizenLabs
constants — Plonky3's `Poseidon2Bn254<3>`) used as a rate-2 sponge, tweak/domain tag in the
capacity element, public parameter `P` absorbed into every call:

  - **stage 1 (encode):** `x = H(P, rho, msg)` [tag: epoch], then a **strict** 254-bit
    decomposition of `x` (booleanity + recomposition + an MSB-first comparison with the
    constant `p`, so `x` and `x+p` cannot both decompose); chunk `x_i` = bits of `x`;
  - **stage 2 (chains):** each chain is unrolled `BASE-1` steps from the revealed
    `sig.hashes[i]`, step `j` hashing `H(P, node)` with tag (epoch, i, **position `x_i + j`**)
    — the absolute position is linear in the chunk bits, so it rides in the capacity element
    for free — and a one-hot selector on the chunk bits picks the chain end at `BASE-1`;
  - **stage 3 (Merkle):** `leaf = H(P, chain_ends)`, folded up the authentication path with
    `H(P, l, r)` [tag: level, position-in-level] using the public epoch-bit directions, then
    `assert(top == pk.root)`.

  **Native side = Plonky3 over FFI.** The honest roots are computed by calling Plonky3's
  permutation through `../plonky3_ffi` (Rust staticlib, C header `p3_poseidon2.h`); the
  gadget's round constants are read from the same library. The gadget itself is written
  independently in C++, so "R1CS satisfied" means gadget == Plonky3. Startup self-tests also
  check gadget == Plonky3 on random states and that the alias check rejects `h+p`. The FFI
  crate's own `cargo test` checks its constants and permutation against the HorizenLabs
  reference (`zkhash`).

  Cost: 240 constraints per permutation (80 S-boxes × 3); everything else in Poseidon2 is
  linear and free in R1CS. Measured (k=4, DIM=4, BASE=4, HGT=3): **24,336 constraints
  (6,084/signer; 23 permutations + ~510 for the strict decomposition), padded to 32,768;
  satisfied; Aurora proof ~151 KB, prove ~7–9 s, verify ~0.6 s, VERIFICATION SUCCESS**;
  tamper check rejects a corrupted revealed chain value.

  **The instance is now REAL core signatures** (see "Poseidon2 tweak hash" below): the
  harness takes an instance file from `export_p2_sigs`, recomputes every root natively and
  checks it equals the signer's public key before building the R1CS. Usage:
  `aggregate_aurora_verify <instance-file> [zk=1|0]`.

  Still a model, not the deployed scheme: plain Winternitz chunks with no checksum /
  target-sum check, and toy parameters (4 chains of w=2, tree height 3).

**3. `bridge_core_to_aurora.cpp` — the REAL core plugged into the SNARK.** Generates k real
Generalized-XMSS signatures with the (now fixed, round-trip-tested) core in `src/signature/`,
verifies each natively, flattens `(pk, sig)` to bytes, and feeds them into program (1). Measured
(k=4): real sigs (pk 48 B, sig 368 B each) → 2048-constraint R1CS satisfied → Aurora proof ~131 KB,
prove 0.38 s / verify 0.03 s, VERIFICATION SUCCESS. The core needs C++23 (concepts/`byteswap`) and
libiop needs C++17, so it builds as two TUs (`core_sigs.cpp` @ C++23, bridge @ C++17) linked
together. This binds the proof to real signature *data*; proving SHA verification *in-circuit* over
real signatures needs the core to sign with Poseidon2 (program (2) proves the Poseidon2 relation).

## Poseidon2 tweak hash + Aurora vs Plonky3 (real signatures)

**The tweak hash (C++ core).** `src/symmetric/tweak_hash/poseidon2.hpp` (`Poseidon2TweakHash`)
and `src/symmetric/message_hash/poseidon2.hpp` (`Poseidon2MessageHash`) instantiate the scheme
with Poseidon2-BN254 — Plonky3's `Poseidon2Bn254<3>`, called through `plonky3_ffi`
(`src/symmetric/poseidon2_bn254.hpp` is the sponge + field glue). `apply(P, tweak, m...)` =
rate-2 sponge over `(P, m...)` with the tweak as a u64 tag in the capacity element. The tags
live in one header, `src/symmetric/tweak_hash/poseidon2_tags.h`, included by both the core and
the Aurora circuit (the Plonky3 AIR mirrors it in `plonky3_air/src/native.rs`). Domain and
Parameter stay `std::vector<uint8_t>` (32-byte LE field elements), so the rest of the core is
unchanged. `src/signature/tests/test_poseidon2.cpp`: 27/27 pass (valid sigs; tampered chain
value, tampered Merkle path, wrong message, wrong epoch all rejected) for a 64+4-chain config
with checksum and for the circuit's demo config.

**One instance, two provers.** `export_p2_sigs <k> <epoch> <file>` signs with the core,
verifies natively, and writes the instance. Both provers read the same file and first check
that their own native hash reproduces every public key:

- **Aurora** — `aurora/aggregate_aurora_verify.cpp` (R1CS, 6,084 constraints/signer).
- **Plonky3** — `plonky3_air/` (uni-stark over BN254, `cargo run --release -- <file> [queries]
  [log_blowup]`). One Poseidon2 permutation per row (each S-box committed as x^3 and x^5:
  160 cols), 23 rows/signer on a fixed schedule held in 56 preprocessed columns (absorb
  sources, sponge continuation, tags, row roles), per-signer registers for values reused
  across rows, the same strict 254-bit decomposition + alias check, max constraint degree 3,
  552 main columns. Commitments are BLAKE3 Merkle trees; Fiat-Shamir is a small BLAKE3
  transcript (`config.rs`) because Plonky3's `DuplexChallenger` only samples bits for 64-bit
  fields. Spot checks per run: corrupted witness → rejected, wrong public key → rejected.

`bench_p2_agg.sh [k...]` runs everything. Both provers are single-threaded here (libiop
build; Plonky3 without the `parallel` feature). i7-10750H, 2026-10-04:

| k  | Aurora zk=1: proof / prove / verify / RAM | Aurora zk=0 | Plonky3 (rate 1/4, 56 q) | Plonky3 (rate 1/32, 23 q) |
|----|-------------------------------------------|-------------|--------------------------|---------------------------|
| 4  | 151 KB / 6.2 s / 0.55 s / 416 MB          | 205 KB / 2.1 s / 0.93 s / 158 MB | 1.29 MB / 0.14 s / 17 ms / 19 MB | 577 KB / 0.88 s / 7 ms / 85 MB |
| 8  | 140 KB / 12.8 s / 0.97 s / 794 MB         | 226 KB / 4.5 s / 1.8 s / 310 MB  | 1.33 MB / 0.30 s / 18 ms / 32 MB | 596 KB / 1.9 s / 7 ms / 167 MB |
| 16 | 178 KB / 27.7 s / 2.2 s / 1.6 GB          | 250 KB / 9.1 s / 3.6 s / 614 MB  | 1.39 MB / 0.66 s / 18 ms / 55 MB | 622 KB / 4.0 s / 8 ms / 338 MB |
| 32 | 193 KB / 56.8 s / 4.4 s / 3.2 GB          | 274 KB / 18.3 s / 7.3 s / 1.2 GB | 1.48 MB / 1.3 s / 19 ms / 108 MB | 661 KB / 8.4 s / 8 ms / 703 MB |

All at ~112-bit conjectured security: Aurora's "128" setting reports *achieved 112.0*
(heuristic, 56 queries at zk=1; 111 queries on a 4x smaller domain at zk=0, hence the bigger
non-zk proof); Plonky3 at log_blowup·queries = 112 (ethSTARK conjecture; intermediate points
q=38/rate 1/8 and q=28/rate 1/16 are in `bench_results_p2.tsv`). Plonky3 is not zero-knowledge.

Reading it:
- **Plonky3 (rate 1/4) proves ~14x faster than Aurora zk=0 (~45x vs zk=1) and verifies 30–400x
  faster, with far less memory** — its trace is small (23 rows/signer) while Aurora works on a 4–8x blown-up domain
  of the whole R1CS.
- **Aurora's proofs are ~3–9x smaller.** Plonky3's size is dominated by opening a full trace
  row per query: (552 + 56) BN254 elements x 32 B x queries ≈ 1.1 MB at 56 queries. That is why
  it barely grows with k, and why trading rate for queries (rate 1/32, 23 q) halves it at ~6x
  the prove time. A narrower layout (e.g. splitting a permutation over several rows) would
  shrink proofs further; not done here.
- Both scale linearly in k for prove time; Aurora's proof grows slowly (log), Plonky3's is
  nearly flat.
- BN254 is the only field both share, which keeps the relation identical; it is not Plonky3's
  sweet spot (31-bit fields + SIMD packing, where the reference hash-sig uses Poseidon2 over
  KoalaBear).

## LaBRADOR (ICICLE) — `labrador/`

`aggregate_labrador.cpp` bit-decomposes the k signatures into a **low-norm** witness
`S ∈ Rq^{r·n}` (LaBRADOR requires a small-norm witness), adds EQ + ConstZero
constraints satisfied by `S`, and runs the LaBRADOR prover/verifier.

Build: see `labrador/README.md` (needs `ingonyama-zk/icicle-labrador`; documents the
one-line `HOST_INLINE` fix required for GCC ≥ 13).

## Scope / honesty

- **Aurora now has a real verification circuit** (`aggregate_aurora_verify.cpp`): encode +
  Winternitz chains + Merkle path are all enforced R1CS constraints, satisfied, Aurora-verified,
  and shown to reject tampered witnesses. The hash is real Poseidon2-BN254 (cross-checked
  against Plonky3 over FFI), the C++ core signs with the same hash, and both Aurora and the
  Plonky3 AIR prove real core signatures. Remaining gap: no checksum/target-sum check in-circuit.
- **LaBRADOR also has a real aggregate-verification relation now** (`labrador/aggregate_labrador_verify.c`,
  reference impl via the Dachshund front end): for each signer `<A_i, sig_i> = root_i` with a short
  norm-bounded signature, where `A_i` is the composed linear chain+Merkle verification map. Run under
  Intel SDE (no AVX512 here): aggregate statement satisfied, tamper check rejects a corrupted signature,
  Dachshund→Labrador total proof **73.48 KB** for k=4. Compromise: the hash is modeled as *linear* (fits
  LaBRADOR's linear constraints); a nonlinear/real hash needs LaBRADOR's quadratic constraints (future work).
- `aggregate_aurora.cpp` (Aurora scaffold) and `aggregate_labrador.cpp` (icicle LaBRADOR) **bind the
  proof to real (pk, sig) data** and exercise the full prove→verify pipeline, but their constraints are
  the system's generic satisfiable relation, not the verification relation.

## What was wrong with the original `aggregate.cpp` / `myR1CS.hpp`

- Called `GeneralizedXMSSSignatureScheme::verify(...)` — a type that does not exist.
- Built the constraint `A.add_term(ver_s - 1)` from a **runtime** native verify bit —
  this proves nothing in-circuit.
- `signature_flatten()` was an empty stub; the returned R1CS used **empty**
  primary/auxiliary inputs (the filled locals were discarded).
- Statement/witness types in `aggregate.cpp` did not match those in `myR1CS.hpp`.
- Depended on the (empty) `libiop` submodule and did not build.
