# Multi-signature aggregation: proof systems

The goal of `src/SNARK/` is to aggregate *k* Generalized-XMSS multi-signatures into
**one** succinct proof "these k signatures all verify for message m at epoch e."
Two transparent, post-quantum proof backends are now wired up and **both build and
verify end-to-end** on CPU (tested on this machine, GCC 13.3, no CUDA toolkit):

| Backend      | Kind                         | Dir                 | Status |
|--------------|------------------------------|---------------------|--------|
| **Aurora**   | hash/FRI zk-STARK (libiop)   | `aurora/`           | ✅ builds, proves, verifies |
| **LaBRADOR** | lattice / Module-SIS (ICICLE)| `labrador/`         | ✅ builds, proves, verifies |

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

Three programs, all over `libff::gf64`, built and run by `aurora/build.sh` (needs a
built libiop; the script documents the one-line `<cstddef>` fix for GCC ≥ 13):

**1. `aggregate_aurora.cpp` — data-bound scaffold.** Maps the flattened statement
(k, epoch, message, k public keys) to the R1CS *primary* input and the flattened k
signatures to the *auxiliary* input, builds a satisfiable R1CS bound to that data, and
runs `aurora_snark_prover` / `aurora_snark_verifier`. The constraints are libiop's
generic multiplicative gadget — it exercises the pipeline but does not encode verification.

**2. `aggregate_aurora_verify.cpp` — the REAL verification circuit.** Arithmetizes the
actual relation from `generalized_xmss.hpp::verify()` as genuine enforced constraints:

  - **stage 1 (encode):** `x = H(param, epoch, rho, message)`, then a full 64-bit
    decomposition (`b·b=b` per bit + recomposition) whose low bits are the Winternitz chunks;
  - **stage 2 (chains):** each chain is unrolled `BASE-1` hash steps from the revealed
    `sig.hashes[i]`, and a **one-hot selector** on the chunk bits picks the chain end at
    the data-dependent position `BASE-1-x_i`;
  - **stage 3 (Merkle):** `leaf = H(chain_ends)`, folded up the authentication path with
    the public epoch-bit directions, then `assert(top == pk.root)`.

  Measured (k=4 signers, DIM=4 chains, BASE=4, HGT=3): **2048-constraint R1CS, satisfied;
  Aurora proof ~131 KB, prove ~0.34 s, verify ~0.03 s, VERIFICATION SUCCESS.** A built-in
  **tamper check** corrupts one witness value and confirms the R1CS then *rejects* it, so
  the constraints provably bite.

  **One compromise — the hash primitive.** A real SHA-256 R1CS gadget is thousands of
  hand-verified constraints and infeasible to author correctly here, so `H` is a
  MiMC-style cubing sponge over gf64 (`state <- (state+input+rc)^3`, R rounds). It is a
  genuine, fully-constrained algebraic hash but **not** SHA-256 — so this demonstrates the
  circuit *structure and constraint logic*, not the real scheme's collision resistance.
  Swapping in a SHA-256/Poseidon gadget is the remaining work; everything around it
  (chains, selectors, Merkle folding, bit/range decomposition, root equality) is real.
  Also note: the per-step tweak uses `(chain_index, step)` rather than absolute
  position-in-chain, a small documented simplification.

**3. `bridge_core_to_aurora.cpp` — the REAL core plugged into the SNARK.** Generates k real
Generalized-XMSS signatures with the (now fixed, round-trip-tested) core in `src/signature/`,
verifies each natively, flattens `(pk, sig)` to bytes, and feeds them into program (1). Measured
(k=4): real sigs (pk 48 B, sig 368 B each) → 2048-constraint R1CS satisfied → Aurora proof ~131 KB,
prove 0.38 s / verify 0.03 s, VERIFICATION SUCCESS. The core needs C++23 (concepts/`byteswap`) and
libiop needs C++17, so it builds as two TUs (`core_sigs.cpp` @ C++23, bridge @ C++17) linked
together. This binds the proof to real signature *data*; proving SHA verification *in-circuit* over
real signatures still needs the SHA gadget (program (2) uses a model hash).

## LaBRADOR (ICICLE) — `labrador/`

`aggregate_labrador.cpp` bit-decomposes the k signatures into a **low-norm** witness
`S ∈ Rq^{r·n}` (LaBRADOR requires a small-norm witness), adds EQ + ConstZero
constraints satisfied by `S`, and runs the LaBRADOR prover/verifier.

Build: see `labrador/README.md` (needs `ingonyama-zk/icicle-labrador`; documents the
one-line `HOST_INLINE` fix required for GCC ≥ 13).

## Scope / honesty

- **Aurora now has a real verification circuit** (`aggregate_aurora_verify.cpp`): encode +
  Winternitz chains + Merkle path are all enforced R1CS constraints, satisfied, Aurora-verified,
  and shown to reject tampered witnesses. The only compromise is the hash primitive (a model
  MiMC sponge, not SHA-256) — swapping in a SHA-256/Poseidon gadget is the remaining work.
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
