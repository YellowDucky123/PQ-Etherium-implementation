/*  aggregate_labrador_verify.c
 *
 *  Aggregate k Generalized-XMSS signature *verifications* into ONE LaBRADOR proof,
 *  using the reference implementation (github.com/lattice-dogs/labrador) via its
 *  Dachshund front end (linear constraints over R_q with a small-norm witness).
 *
 *  THE RELATION (this is the point):
 *    For each signer i, verification is  root_i = Verify(sig_i).  Under a *linear*
 *    model hash H(x)=c*x (and 2->1 node H(a,b)=m_n*a+m_s*b), the whole
 *      encode -> Winternitz chains -> Merkle path -> root
 *    computation is a *linear* map A_i applied to the signature, so verification
 *    becomes exactly LaBRADOR's native relation:
 *
 *        prover knows a SHORT sig_i  with  <A_i, sig_i> = root_i     (for all i)
 *
 *    - sig_i  = the signature = [ revealed chain values (DIM) | Merkle path sibs (HGT) | pad ]
 *               a short (ternary) witness vector, exactly what LaBRADOR proves knowledge of.
 *    - A_i    = the PUBLIC verification map, whose entries are the composed linear
 *               chain+Merkle multipliers:
 *                 chain c : m_n^HGT * m_leaf * c_step^(BASE-1 - x_c)   (x_c public, from msg)
 *                 sib   l : m_sib  * m_n^(HGT-1-l)
 *               i.e. the coefficient tells you how many chain steps / how many Merkle
 *               levels that signature element passes through -- the real structure.
 *    - root_i = <A_i, sig_i>  (the public commitment / tree root).
 *
 *  So the proof genuinely certifies "these k signatures all verify" (their roots are
 *  the correct linear image of the secret short signatures), aggregated into one
 *  LaBRADOR proof, then recursed once more with the base Labrador prover.
 *
 *  HONESTY: the hash is modeled as a *linear* map (chosen so the exact XMSS
 *  verification structure fits LaBRADOR's linear-constraint relation and is provable
 *  today). A linear hash is not collision resistant; making it a real
 *  SHA-256/Poseidon means arithmetizing the hash into LaBRADOR's quadratic
 *  constraints -- the remaining research work. The relation FORM here (k short-preimage
 *  / opening constraints binding public roots to secret signatures, with a norm bound)
 *  is the correct aggregate-verification relation.
 *
 *  Build + run: see build.sh in this directory (requires AVX512; run under Intel SDE
 *  on non-AVX512 hardware, e.g.  sde64 -icx -- ./aggregate_labrador_verify).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "dachshund.h"
#include "labrador.h"
#include "poly.h"        /* N, polyvec_fromint64vec, polyvec_sprodz */

/* ----------------------------- demo parameters ----------------------------- */
#define K_SIGNERS 4
#define DIM       8          /* Winternitz chains per signer         */
#define HGT       5          /* Merkle tree height (lifetime 2^HGT)  */
#define W_BITS    2
#define BASE      (1 << W_BITS)   /* chain length                    */
#define MEANING   (DIM + HGT)     /* meaningful signature elements   */
#define M         512        /* witness rank per signer (rest is signature padding) */

/* linear model-hash scalar multipliers */
#define CHAIN_MUL 3
#define LEAF_MUL  1
#define NODE_MUL  2
#define SIB_MUL   1

static int64_t ipow(int64_t b, int e) { int64_t r = 1; while (e-- > 0) r *= b; return r; }

/* tiny deterministic ternary PRNG in {-1,0,1} */
static uint64_t rng_state = 0x9e3779b97f4a7c15ULL;
static int64_t ternary(void) {
    rng_state ^= rng_state << 13; rng_state ^= rng_state >> 7; rng_state ^= rng_state << 17;
    return (int64_t)(rng_state % 3) - 1;
}

/* Public verification map A[k] for one signer (same public params/message for all,
 * different signatures -> different roots). x_c derived from a public "message". */
static void build_map(int64_t A[M]) {
    for (int k = 0; k < M; ++k) A[k] = 0;
    int64_t leaf_climb = ipow(NODE_MUL, HGT);          /* leaf passes all HGT levels */
    for (int c = 0; c < DIM; ++c) {
        int x_c   = (c * 7 + 3) % BASE;                /* public chunk from message  */
        int steps = (BASE - 1) - x_c;                  /* forward chain steps        */
        A[c] = leaf_climb * LEAF_MUL * ipow(CHAIN_MUL, steps);
    }
    for (int l = 0; l < HGT; ++l)
        A[DIM + l] = SIB_MUL * ipow(NODE_MUL, HGT - 1 - l);
}

/* Build the aggregate-verification statement + witness. */
static void prepare(smplstmnt *st, witness *wt, int tamper) {
    size_t n[K_SIGNERS];
    uint64_t betasq[K_SIGNERS];
    for (int i = 0; i < K_SIGNERS; ++i) { n[i] = M; betasq[i] = (uint64_t)M * N; } /* ternary max norm */

    int64_t A[M];
    build_map(A);

    init_witness_raw(wt, K_SIGNERS, n);
    init_smplstmnt_raw(st, K_SIGNERS, n, betasq, K_SIGNERS);

    static int64_t sig[M * N];   /* one signer's signature coefficients */
    static int64_t phi[M * N];   /* the public map A as constant polys  */
    static int64_t b[N];         /* the root = <A, sig>                 */

    for (int i = 0; i < K_SIGNERS; ++i) {
        /* sample a short (ternary) signature */
        for (int k = 0; k < M; ++k)
            for (int j = 0; j < N; ++j)
                sig[k * N + j] = ternary();

        /* compute the public root  b = <A, sig>  (A entries are scalars -> constant polys) */
        for (int j = 0; j < N; ++j) {
            int64_t acc = 0;
            for (int k = 0; k < M; ++k) acc += A[k] * sig[k * N + j];
            b[j] = acc;
        }

        /* load the signature as this signer's witness vector */
        polyvec_fromint64vec(wt->s[i], M, 1, sig);
        wt->normsq[i] = polyvec_sprodz(wt->s[i], wt->s[i], M);

        if (tamper && i == 0) sig[0] += 1;   /* corrupt sig AFTER root fixed -> must break */
        (void)0;

        /* phi = A written as M constant polynomials (coeff 0 = A[k], rest 0) */
        for (int k = 0; k < M; ++k) { phi[k * N] = A[k]; for (int j = 1; j < N; ++j) phi[k * N + j] = 0; }

        if (tamper && i == 0) {
            /* reload the CORRUPTED signature into the witness, keep original root b */
            polyvec_fromint64vec(wt->s[i], M, 1, sig);
            wt->normsq[i] = polyvec_sprodz(wt->s[i], wt->s[i], M);
        }

        size_t idx[1] = { (size_t)i }, nn[1] = { M };
        set_smplstmnt_lincnst_raw(st, i, 1, idx, nn, 1, phi, b);
    }
}

int main(void) {
    int ret;
    commitment com = {};
    smplstmnt st0 = {};
    statement  st1 = {}, st2 = {};
    proof      pi0 = {}, pi1 = {};
    witness    wt0 = {}, wt1 = {}, wt2 = {};
    double size = 0;

    printf("Aggregating k=%d XMSS signature verifications via LaBRADOR (Dachshund)\n", K_SIGNERS);
    printf("  per signer: DIM=%d chains, HGT=%d Merkle levels, witness rank M=%d (ternary)\n\n",
           DIM, HGT, M);

    /* --- soundness sanity: a tampered signature must be REJECTED --- */
    {
        smplstmnt sx = {}; witness wx = {};
        prepare(&sx, &wx, /*tamper=*/1);
        int bad = simple_verify(&sx, &wx);
        printf("[labrador-verify] tamper check (corrupt a signature): statement %s (expected rejected)\n\n",
               bad ? "rejected" : "ACCEPTED - BUG!");
        free_smplstmnt(&sx); free_witness(&wx);
    }

    prepare(&st0, &wt0, /*tamper=*/0);
    print_smplstmnt_pp(&st0);
    ret = simple_verify(&st0, &wt0);
    if (ret) { fprintf(stderr, "ERROR: aggregate statement not satisfied: %d\n", ret); goto end; }
    printf("[labrador-verify] aggregate statement satisfied: all %d verifications hold.\n\n", K_SIGNERS);

    /* Dachshund -> reduces the linear statement to a Labrador statement */
    simple_prove(&st1, &wt1, &pi0, &com, &st0, &wt0, 0);
    size += print_proof_pp(&pi0);
    ret = verify(&st1, &wt1);
    if (ret) { fprintf(stderr, "ERROR: Dachshund verify failed: %d\n", ret); goto end; }
    free_statement(&st1);
    ret = simple_reduce(&st1, &pi0, &com, &st0);
    if (ret) { fprintf(stderr, "ERROR: simple_reduce failed: %d\n", ret); goto end; }
    ret = verify(&st1, &wt1);
    if (ret) { fprintf(stderr, "ERROR: verify after simple_reduce failed: %d\n", ret); goto end; }

    /* one more Labrador layer */
    prove(&st2, &wt2, &pi1, &st1, &wt1, 0);
    size += print_proof_pp(&pi1);
    ret = verify(&st2, &wt2);
    if (ret) { fprintf(stderr, "ERROR: Labrador verify failed: %d\n", ret); goto end; }
    free_statement(&st2);
    ret = reduce(&st2, &pi1, &st1);
    if (ret) { fprintf(stderr, "ERROR: reduce failed: %d\n", ret); goto end; }
    ret = verify(&st2, &wt2);
    if (ret) { fprintf(stderr, "ERROR: verify after reduce failed: %d\n", ret); goto end; }

    size += print_witness_pp(&wt2);
    printf("\n[labrador-verify] AGGREGATE VERIFICATION PROOF OK. Total proof size: %.2f KB\n", size);

end:
    free_smplstmnt(&st0); free_statement(&st1); free_statement(&st2);
    free_proof(&pi0); free_proof(&pi1);
    free_witness(&wt0); free_witness(&wt1); free_witness(&wt2);
    free_comkey();
    return ret;
}
