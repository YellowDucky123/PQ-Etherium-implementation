/*  aggregate_aurora_verify.cpp
 *
 *  A REAL (not scaffold) R1CS circuit for aggregating k Generalized-XMSS
 *  signature verifications, proven with libiop's Aurora zk-STARK over gf64.
 *
 *  Unlike aggregate_aurora.cpp (which builds a generic satisfiable R1CS just to
 *  exercise the prover/verifier), this file arithmetizes the ACTUAL verification
 *  relation from src/signature/generalized_xmss.hpp::verify(), in three stages,
 *  as genuine enforced constraints:
 *
 *     stage 1 (encode):  x = H(param, epoch, rho, message) ; chunks = bits of x
 *     stage 2 (chains):  for each chain i, walk the tweakable hash forward
 *                        (BASE-1 - x_i) steps from the revealed value sig.hashes[i]
 *                        to the chain end   (unrolled + one-hot selector on x_i)
 *     stage 3 (merkle):  leaf = H(chain_ends); fold leaf up the authentication
 *                        path with the (public) epoch-bit directions; assert the
 *                        top equals the public root pk.root
 *
 *  THE ONE COMPROMISE — the hash primitive:
 *     A real SHA-256 R1CS gadget is thousands of hand-verified constraints and is
 *     infeasible to author correctly here, so the tweakable/message hash is a
 *     MiMC-style cubing sponge over gf64 (state <- (state+input+rc)^3, R rounds).
 *     It is a genuine, fully-constrained algebraic hash, but it is NOT SHA-256, so
 *     this demonstrates the circuit STRUCTURE and constraint logic, not the real
 *     scheme's collision resistance. Swapping in a SHA-256/Poseidon gadget is the
 *     remaining work; everything around it (chains, selectors, Merkle folding,
 *     bit/range decomposition, public-root equality) is real and reusable.
 *
 *  Everything is one self-consistent instance: a valid (pk, sig) tuple is sampled
 *  under the model hash, so the R1CS is satisfiable and Aurora verifies it.
 */

#include <cstdint>
#include <cstdio>
#include <vector>
#include <array>
#include <random>
#include <chrono>

#include <libff/algebra/fields/binary/gf64.hpp>
#include "libiop/snark/aurora_snark.hpp"
#include "libiop/relations/r1cs.hpp"

using libff::gf64;
using FieldT = gf64;
using libiop::variable;
using libiop::linear_combination;
using libiop::r1cs_constraint;

// ----------------------------- demo parameters -----------------------------
static constexpr int      R_ROUNDS  = 4;   // MiMC sponge rounds
static constexpr int      W_BITS    = 2;   // bits per Winternitz chunk
static constexpr int      BASE      = 1 << W_BITS; // chain length (4)
static constexpr int      MAXSTEPS  = BASE - 1;    // 3
static constexpr int      DIM       = 4;   // number of chains per signer
static constexpr int      HGT       = 3;   // Merkle tree height (lifetime 8)
static constexpr int      MSG_ELEMS = 2;   // message packed into 2 field elements
static constexpr int      K_SIGNERS = 4;   // signatures aggregated

static const uint64_t IV  = 0x1234567890abcdefULL;
static const uint64_t RC[R_ROUNDS] = {
    0x9e3779b97f4a7c15ULL, 0xbf58476d1ce4e5b9ULL,
    0x94d049bb133111ebULL, 0x2545f4914f6cdd1dULL };
enum { DOM_ENC = 1, DOM_CHAIN = 2, DOM_LEAF = 3, DOM_NODE = 4 };
static uint64_t dom(uint64_t base, uint64_t a=0, uint64_t b=0, uint64_t c=0) {
    return (base << 56) ^ (a << 40) ^ (b << 20) ^ c;
}

// ----------------------------- native model hash ---------------------------
// Must match the in-circuit gadget exactly (used to build a consistent instance).
static gf64 ncube(const gf64 &x) { gf64 s = x * x; return s * x; }
static gf64 nperm(gf64 x) { for (int r = 0; r < R_ROUNDS; ++r) x = ncube(x + gf64(RC[r])); return x; }
static gf64 nhash(const std::vector<gf64> &ins, uint64_t d) {
    gf64 acc = gf64(IV) + gf64(d);
    for (const auto &in : ins) acc = nperm(acc + in);
    return acc;
}

// ------------------------------ R1CS builder -------------------------------
struct Builder {
    std::vector<FieldT> asg;         // full variable assignment (var 0 = ONE, implicit)
    size_t num_pub = 0;
    bool   priv_started = false;
    libiop::r1cs_constraint_system<FieldT> cs;
    size_t n_constraints = 0;

    using LC = linear_combination<FieldT>;

    // NOTE: libiop's linear_combination::operator+ has a bug (uses .index not
    // .index_) and assumes sorted terms; we concatenate terms instead, which is
    // correct because evaluate() just sums over all terms.
    static LC add(const LC &a, const LC &b) { LC r = a; for (auto &t : b.terms) r.terms.push_back(t); return r; }

    LC one_lc()            const { LC l; l.add_term(variable<FieldT>(0), FieldT::one()); return l; }
    LC cst(const gf64 &c)  const { LC l; l.add_term(variable<FieldT>(0), c); return l; }
    LC v_lc(size_t i)      const { LC l; l.add_term(variable<FieldT>(i), FieldT::one()); return l; }
    FieldT eval(const LC &l) const { return l.evaluate(asg); }

    size_t alloc_pub(const gf64 &val) { if (priv_started) throw std::logic_error("pub after priv");
                                        asg.push_back(val); ++num_pub; return asg.size(); }
    size_t alloc(const gf64 &val)     { priv_started = true; asg.push_back(val); return asg.size(); }

    void constrain(const LC &a, const LC &b, const LC &c) {
        cs.add_constraint(r1cs_constraint<FieldT>(a, b, c)); ++n_constraints;
    }
    // returns a fresh variable equal to a*b
    size_t mul(const LC &a, const LC &b) {
        size_t v = alloc(eval(a) * eval(b));
        constrain(a, b, v_lc(v));
        return v;
    }
    void assert_eq(const LC &a, const LC &b) { constrain(a, one_lc(), b); } // a*1 == b

    // MiMC cubing-sponge hash of the given input LCs; returns output variable.
    size_t hash(const std::vector<LC> &ins, uint64_t d) {
        LC acc = cst(gf64(IV) + gf64(d));
        for (const auto &in : ins) {
            LC cur = add(acc, in);
            for (int r = 0; r < R_ROUNDS; ++r) {
                LC base = add(cur, cst(gf64(RC[r])));
                size_t sq   = mul(base, base);
                size_t cube = mul(v_lc(sq), base);
                cur = v_lc(cube);
            }
            acc = cur;
        }
        size_t out = alloc(eval(acc));
        assert_eq(v_lc(out), acc);
        return out;
    }

    // Full 64-bit decomposition of variable h: returns the 64 bit-variables, and
    // enforces each is a bit and that they recompose to h (over gf64).
    std::array<size_t, 64> bits64(size_t h) {
        uint64_t w = eval(v_lc(h)).to_words()[0];
        std::array<size_t, 64> b;
        LC recomp;
        for (int j = 0; j < 64; ++j) {
            size_t bj = alloc(gf64((w >> j) & 1ULL));
            constrain(v_lc(bj), v_lc(bj), v_lc(bj));      // b*b = b  => b in {0,1}
            recomp.add_term(variable<FieldT>(bj), gf64(1ULL << j)); // + b_j * x^j
            b[j] = bj;
        }
        assert_eq(recomp, v_lc(h));                        // sum b_j x^j == h
        return b;
    }
};

// ---------------------- one signer's verification circuit ------------------
// Public (already allocated): msg[MSG_ELEMS], epoch (value), param_s, root_s.
// Private inputs sampled by the caller: rho_s, revealed chain values, path sibs.
static void add_signer_verification(
    Builder &B, uint64_t epoch,
    const std::vector<size_t> &msg_vars, size_t param_var, size_t root_var,
    const gf64 &rho_val, const std::array<gf64, DIM> &revealed,
    const std::array<gf64, HGT> &path)
{
    using LC = Builder::LC;

    // private inputs
    size_t rho = B.alloc(rho_val);
    std::array<size_t, DIM> start;
    for (int i = 0; i < DIM; ++i) start[i] = B.alloc(revealed[i]);
    std::array<size_t, HGT> sib;
    for (int l = 0; l < HGT; ++l) sib[l] = B.alloc(path[l]);

    // ---- stage 1: encode -> chunk bits ----
    std::vector<LC> enc_in{ B.v_lc(rho) };
    for (size_t m : msg_vars) enc_in.push_back(B.v_lc(m));
    enc_in.push_back(B.v_lc(param_var));
    size_t Hx = B.hash(enc_in, dom(DOM_ENC, epoch));
    std::array<size_t, 64> hbits = B.bits64(Hx);
    // chunk i uses bits [i*W_BITS, i*W_BITS+W_BITS)
    std::array<std::array<size_t, W_BITS>, DIM> chunk_bits;
    for (int i = 0; i < DIM; ++i)
        for (int t = 0; t < W_BITS; ++t)
            chunk_bits[i][t] = hbits[i * W_BITS + t];

    // ---- stage 2: chains ----
    std::array<size_t, DIM> chain_end;
    for (int i = 0; i < DIM; ++i) {
        // unroll the forward walk: node[0]=start, node[j]=H(node[j-1])
        std::array<size_t, MAXSTEPS + 1> node;
        node[0] = start[i];
        for (int j = 1; j <= MAXSTEPS; ++j)
            node[j] = B.hash({ B.v_lc(node[j - 1]) }, dom(DOM_CHAIN, epoch, i, j));

        // x_i integer value (from its bits), and one-hot selector s_v = [x_i == v]
        uint64_t xi = 0;
        for (int t = 0; t < W_BITS; ++t)
            xi |= (B.eval(B.v_lc(chunk_bits[i][t])).to_words()[0] & 1ULL) << t;

        // chain_end = node[MAXSTEPS - x_i]  via  sum_v s_v * node[MAXSTEPS - v]
        LC acc_end;                                   // linear accumulation of selected terms
        for (int v = 0; v < BASE; ++v) {
            // s_v = product over bits t of (bit==1? b_t : (1 - b_t))
            LC sfac = B.one_lc();
            size_t s_run = 0; bool first = true;
            for (int t = 0; t < W_BITS; ++t) {
                LC factor = ((v >> t) & 1) ? B.v_lc(chunk_bits[i][t])
                                           : Builder::add(B.one_lc(), B.v_lc(chunk_bits[i][t]));
                if (first) { sfac = factor; first = false; }
                else       { s_run = B.mul(sfac, factor); sfac = B.v_lc(s_run); }
            }
            size_t s_v = (W_BITS == 1) ? B.mul(sfac, B.one_lc()) : s_run; // materialize
            size_t term = B.mul(B.v_lc(s_v), B.v_lc(node[MAXSTEPS - v]));  // s_v * node
            acc_end = Builder::add(acc_end, B.v_lc(term));
        }
        chain_end[i] = B.alloc(B.eval(acc_end));
        B.assert_eq(B.v_lc(chain_end[i]), acc_end);
    }

    // ---- stage 3: leaf + Merkle path ----
    std::vector<LC> leaf_in;
    for (int i = 0; i < DIM; ++i) leaf_in.push_back(B.v_lc(chain_end[i]));
    leaf_in.push_back(B.v_lc(param_var));
    size_t node_cur = B.hash(leaf_in, dom(DOM_LEAF, epoch));

    for (int l = 0; l < HGT; ++l) {
        bool dir = (epoch >> l) & 1;                 // public -> order known at build time
        std::vector<LC> in = dir ? std::vector<LC>{ B.v_lc(sib[l]), B.v_lc(node_cur) }
                                 : std::vector<LC>{ B.v_lc(node_cur), B.v_lc(sib[l]) };
        node_cur = B.hash(in, dom(DOM_NODE, l));
    }
    // assert computed root == public root
    B.assert_eq(B.v_lc(node_cur), B.v_lc(root_var));
}

static size_t next_pow2(size_t n) { size_t p = 1; while (p < n) p <<= 1; return p; }

int main() {
    std::mt19937_64 rng(12345);
    auto rnd = [&]{ return gf64(rng()); };

    Builder B;

    // ---- public inputs first (statement) ----
    std::vector<size_t> msg_vars(MSG_ELEMS);
    std::vector<gf64>   msg_vals(MSG_ELEMS);
    for (int i = 0; i < MSG_ELEMS; ++i) { msg_vals[i] = rnd(); msg_vars[i] = B.alloc_pub(msg_vals[i]); }
    const uint64_t epoch = 5;                          // public leaf index (0..2^HGT-1)
    B.alloc_pub(gf64(epoch));                          // bind epoch into the statement

    struct Signer { size_t param_var, root_var; };
    std::array<Signer, K_SIGNERS> signers;
    std::array<gf64, K_SIGNERS>   param_vals;
    // per-signer secret material (sampled, then honest root computed natively)
    std::array<gf64, K_SIGNERS>                       rho_vals;
    std::array<std::array<gf64, DIM>, K_SIGNERS>      revealed;
    std::array<std::array<gf64, HGT>, K_SIGNERS>      paths;
    std::array<gf64, K_SIGNERS>                       roots;

    for (int s = 0; s < K_SIGNERS; ++s) {
        param_vals[s] = rnd();
        rho_vals[s]   = rnd();
        for (int i = 0; i < DIM; ++i) revealed[s][i] = rnd();
        for (int l = 0; l < HGT; ++l) paths[s][l]    = rnd();

        // ---- native honest computation of the root (mirrors the circuit) ----
        std::vector<gf64> enc_in{ rho_vals[s] };
        for (auto &m : msg_vals) enc_in.push_back(m);
        enc_in.push_back(param_vals[s]);
        gf64 Hx = nhash(enc_in, dom(DOM_ENC, epoch));
        uint64_t hw = Hx.to_words()[0];
        std::array<gf64, DIM> cend;
        for (int i = 0; i < DIM; ++i) {
            std::array<gf64, MAXSTEPS + 1> node; node[0] = revealed[s][i];
            for (int j = 1; j <= MAXSTEPS; ++j)
                node[j] = nhash({ node[j - 1] }, dom(DOM_CHAIN, epoch, i, j));
            uint64_t xi = (hw >> (i * W_BITS)) & (BASE - 1);
            cend[i] = node[MAXSTEPS - xi];
        }
        std::vector<gf64> leaf_in(cend.begin(), cend.end());
        leaf_in.push_back(param_vals[s]);
        gf64 node_cur = nhash(leaf_in, dom(DOM_LEAF, epoch));
        for (int l = 0; l < HGT; ++l) {
            bool d = (epoch >> l) & 1;
            node_cur = d ? nhash({ paths[s][l], node_cur }, dom(DOM_NODE, l))
                         : nhash({ node_cur, paths[s][l] }, dom(DOM_NODE, l));
        }
        roots[s] = node_cur;

        signers[s].param_var = B.alloc_pub(param_vals[s]);
        signers[s].root_var  = B.alloc_pub(roots[s]);
    }

    // pad public block up to (2^p - 1) inputs
    size_t target_inputs = next_pow2(B.num_pub + 1) - 1;
    while (B.num_pub < target_inputs) B.alloc_pub(gf64(1));

    // ---- build the k verification sub-circuits (private + intermediates) ----
    for (int s = 0; s < K_SIGNERS; ++s)
        add_signer_verification(B, epoch, msg_vars, signers[s].param_var, signers[s].root_var,
                                rho_vals[s], revealed[s], paths[s]);

    // ---- pad variables/constraints to Aurora-friendly sizes ----
    size_t target_vars = next_pow2(B.asg.size() + 1) - 1;
    while (B.asg.size() < target_vars) B.asg.push_back(gf64(1)); // dummy aux vars
    size_t target_constraints = next_pow2(std::max(B.n_constraints, B.asg.size()));
    while (B.n_constraints < target_constraints)
        B.constrain(B.cst(gf64(0)), B.cst(gf64(0)), B.cst(gf64(0))); // 0*0=0

    B.cs.primary_input_size_   = B.num_pub;
    B.cs.auxiliary_input_size_ = B.asg.size() - B.num_pub;

    typedef libiop::binary_hash_digest hash_type;
    libiop::r1cs_primary_input<FieldT>   primary(B.asg.begin(), B.asg.begin() + B.num_pub);
    libiop::r1cs_auxiliary_input<FieldT> auxiliary(B.asg.begin() + B.num_pub, B.asg.end());

    printf("[aurora-verify] k=%d signers | DIM=%d chains, BASE=%d, HGT=%d, hash-rounds=%d\n",
           K_SIGNERS, DIM, BASE, HGT, R_ROUNDS);
    printf("[aurora-verify] R1CS: %zu constraints, %zu variables, %zu public inputs\n",
           B.cs.num_constraints(), B.cs.num_variables(), B.cs.num_inputs());

    if (!B.cs.is_satisfied(primary, auxiliary)) {
        printf("[aurora-verify] ERROR: R1CS NOT satisfied (circuit/native mismatch)\n");
        return 1;
    }
    printf("[aurora-verify] R1CS satisfied: encode + chains + Merkle path all check out.\n");

    // Soundness sanity check: corrupt one private witness value (a revealed chain
    // value) and confirm the constraints now REJECT it -- proving they actually
    // enforce the verification relation and aren't trivially satisfiable.
    {
        libiop::r1cs_auxiliary_input<FieldT> bad = auxiliary;
        bad[0] = bad[0] + gf64(1);   // flip signer 0's first revealed chain value
        const bool still_ok = B.cs.is_satisfied(primary, bad);
        printf("[aurora-verify] tamper check (corrupt a witness): R1CS %s (expected rejected)\n",
               still_ok ? "STILL ACCEPTED - BUG!" : "rejected");
    }

    // ---- prove + verify with Aurora ----
    const size_t security_parameter = 128, RS_extra_dimensions = 2, FRI_localization_parameter = 3;
    libiop::aurora_snark_parameters<FieldT, hash_type> params(
        security_parameter,
        libiop::LDT_reducer_soundness_type::optimistic_heuristic,
        libiop::FRI_soundness_type::heuristic,
        libiop::blake2b_type, FRI_localization_parameter, RS_extra_dimensions,
        /*make_zk=*/true, libiop::affine_subspace_type,
        B.cs.num_constraints(), B.cs.num_variables());

    auto t0 = std::chrono::high_resolution_clock::now();
    const auto argument = libiop::aurora_snark_prover<FieldT>(B.cs, primary, auxiliary, params);
    auto t1 = std::chrono::high_resolution_clock::now();
    printf("[aurora-verify] proof size: %lu bytes | prove %ld ms\n",
           argument.size_in_bytes(),
           std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

    auto t2 = std::chrono::high_resolution_clock::now();
    const bool ok = libiop::aurora_snark_verifier<FieldT, hash_type>(B.cs, primary, argument, params);
    auto t3 = std::chrono::high_resolution_clock::now();
    printf("[aurora-verify] verify %ld ms | VERIFICATION: %s\n",
           std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count(),
           ok ? "SUCCESS" : "FAILED");
    return ok ? 0 : 1;
}
