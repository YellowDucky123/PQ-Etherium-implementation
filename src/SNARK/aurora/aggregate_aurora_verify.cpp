/*  aggregate_aurora_verify.cpp
 *
 *  A REAL (not scaffold) R1CS circuit for aggregating k Generalized-XMSS
 *  signature verifications, proven with libiop's Aurora zk-SNARK over the BN254
 *  scalar field (alt_bn128 Fr), with POSEIDON2 as the hash.
 *
 *  It arithmetizes the verification relation from
 *  src/signature/generalized_xmss.hpp::verify(), in three stages:
 *
 *     stage 1 (encode):  x = H(param, rho, msg)            [tag: epoch]
 *                        strict 254-bit decomposition of x; chunk x_i = bits of x
 *     stage 2 (chains):  for each chain i, walk forward from the revealed value
 *                        sig.hashes[i] (at position x_i) to position BASE-1:
 *                        node_j = H(param, node_{j-1})   [tag: epoch, i, x_i + j]
 *                        (unrolled MAXSTEPS times + one-hot selector on x_i)
 *     stage 3 (merkle):  leaf = H(param, chain_ends)       [tag: epoch]
 *                        fold up the authentication path with the public epoch-bit
 *                        directions, node = H(param, l, r) [tag: level, position];
 *                        assert the top equals the public root pk.root
 *
 *  THE HASH: Poseidon2 over BN254, t=3, x^5, R_F=8, R_P=56, HorizenLabs constants —
 *  the instance Plonky3 ships as `Poseidon2Bn254<3>`. The NATIVE side calls Plonky3
 *  itself over FFI (../plonky3_ffi, a Rust staticlib); the round constants the
 *  gadget uses are read from that same library. The R1CS gadget is written
 *  independently here, so "R1CS satisfied" means the gadget agrees with Plonky3.
 *  Hashing is a sponge (rate 2, capacity 1): the domain/tweak tag goes in the
 *  capacity element, inputs are absorbed two at a time, output = state[0].
 *
 *  Each Poseidon2 permutation costs 80 S-boxes x 3 constraints (x2, x4, x5) + 1;
 *  round constants, the external/internal linear layers and sponge absorption are
 *  linear and fold into the linear combinations for free.
 *
 *  THE INSTANCE: real signatures from the C++ core (Poseidon2TweakHash +
 *  Poseidon2MessageHash, src/symmetric/), read from a file written by
 *  ../export_p2_sigs. Before building the R1CS the harness recomputes every root
 *  natively from the signature and checks it equals the signer's public key, so the
 *  core's hash and the circuit's hash are the same function. Tweak tags come from the
 *  shared header src/symmetric/tweak_hash/poseidon2_tags.h.
 *
 *  What is still a model (not the deployed scheme): the encoding is plain Winternitz
 *  chunks with no checksum / target-sum check, and the parameters are toy-sized
 *  (4 chains of w=2, tree height 3).
 *
 *  Usage: aggregate_aurora_verify <instance-file> [zk=1|0]
 */

#include <cstdint>
#include <cstdio>
#include <vector>
#include <array>
#include <map>
#include <random>
#include <chrono>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include "libiop/snark/aurora_snark.hpp"
#include "libiop/relations/r1cs.hpp"

#include "p3_poseidon2.h"
#include "../../symmetric/tweak_hash/poseidon2_tags.h"

using FieldT = libff::alt_bn128_Fr;
using libiop::variable;
using libiop::linear_combination;
using libiop::r1cs_constraint;

// ----------------------------- demo parameters -----------------------------
static constexpr int W_BITS    = 2;            // bits per Winternitz chunk
static constexpr int BASE      = 1 << W_BITS;  // chain length (4)
static constexpr int MAXSTEPS  = BASE - 1;     // 3
static constexpr int DIM       = 4;            // number of chains per signer
static constexpr int HGT       = 3;            // Merkle tree height (lifetime 8)
static constexpr int MSG_ELEMS = 2;            // message packed into 2 field elements
static constexpr int FBITS     = 254;          // p < 2^254

// Poseidon2-BN254 instance (checked against the FFI library at startup)
static constexpr int T = 3, RATE = 2, RF = 8, RP = 56, NROWS = RF + RP;


// --------------------------- field <-> u64 limbs ----------------------------
static FieldT from_u64(uint64_t v) { libff::bigint<4> b; b.data[0] = v; return FieldT(b); }
static FieldT from_limbs(const uint64_t *l) {
    libff::bigint<4> b; for (int i = 0; i < 4; ++i) b.data[i] = l[i]; return FieldT(b);
}
static void to_limbs(const FieldT &x, uint64_t *l) {
    const libff::bigint<4> b = x.as_bigint(); for (int i = 0; i < 4; ++i) l[i] = b.data[i];
}

// ------------------------ Poseidon2 via Plonky3 (FFI) -----------------------
static std::array<std::array<FieldT, T>, NROWS> RC;   // HorizenLabs RC3, from Plonky3 side

static void load_poseidon2_from_plonky3() {
    uint32_t prm[4];
    p3_poseidon2_bn254_params(prm);
    if (prm[0] != T || prm[1] != 5 || prm[2] != RF || prm[3] != RP)
        throw std::runtime_error("Plonky3 Poseidon2 instance does not match the gadget");
    std::vector<uint64_t> raw(NROWS * T * 4);
    p3_poseidon2_bn254_round_constants(raw.data());
    for (int r = 0; r < NROWS; ++r)
        for (int c = 0; c < T; ++c) RC[r][c] = from_limbs(&raw[(r * T + c) * 4]);
}

static void p3_permute(std::array<FieldT, T> &s) {
    uint64_t buf[T * 4];
    for (int i = 0; i < T; ++i) to_limbs(s[i], buf + 4 * i);
    if (p3_poseidon2_bn254_permute(buf) != 0) throw std::runtime_error("p3 permute failed");
    for (int i = 0; i < T; ++i) s[i] = from_limbs(buf + 4 * i);
}

// Native sponge hash (Plonky3 permutation). The R1CS gadget below mirrors it.
static FieldT nhash(const std::vector<FieldT> &ins, const FieldT &tag) {
    std::array<FieldT, T> s{ FieldT::zero(), FieldT::zero(), tag };
    for (size_t k = 0; k < ins.size(); k += RATE) {
        for (int j = 0; j < RATE && k + j < ins.size(); ++j) s[j] += ins[k + j];
        p3_permute(s);
    }
    return s[0];
}

// ------------------------------ R1CS builder -------------------------------
// Linear combinations are kept as merged {var -> coeff} maps (var 0 = ONE). This
// matters for Poseidon2: the partial rounds keep state[1], state[2] symbolic, and
// without merging repeated terms their size would grow ~3x per round.
struct Lin {
    std::map<size_t, FieldT> t;
    Lin() = default;
    static Lin var(size_t i, const FieldT &c = FieldT::one()) { Lin l; l.t[i] = c; return l; }
    static Lin cst(const FieldT &c) { return var(0, c); }
    Lin &operator+=(const Lin &o) {
        for (auto &[k, v] : o.t) { auto it = t.find(k); if (it == t.end()) t[k] = v; else it->second += v; }
        return *this;
    }
    friend Lin operator+(Lin a, const Lin &b) { a += b; return a; }
    friend Lin operator-(Lin a, const Lin &b) { a += b * (-FieldT::one()); return a; }
    friend Lin operator*(Lin a, const FieldT &c) { for (auto &[k, v] : a.t) v *= c; return a; }
};

struct Builder {
    std::vector<FieldT> asg;         // full assignment, asg[i-1] = value of variable i
    size_t num_pub = 0;
    bool   priv_started = false;
    libiop::r1cs_constraint_system<FieldT> cs;
    size_t n_constraints = 0;

    FieldT eval(const Lin &l) const {
        FieldT acc = FieldT::zero();
        for (auto &[k, v] : l.t) acc += v * (k == 0 ? FieldT::one() : asg[k - 1]);
        return acc;
    }
    static linear_combination<FieldT> to_lc(const Lin &l) {
        linear_combination<FieldT> lc;
        for (auto &[k, v] : l.t) if (!v.is_zero()) lc.add_term(variable<FieldT>(k), v);
        return lc;
    }

    size_t alloc_pub(const FieldT &val) {
        if (priv_started) throw std::logic_error("pub after priv");
        asg.push_back(val); ++num_pub; return asg.size();
    }
    size_t alloc(const FieldT &val) { priv_started = true; asg.push_back(val); return asg.size(); }

    void constrain(const Lin &a, const Lin &b, const Lin &c) {
        cs.add_constraint(r1cs_constraint<FieldT>(to_lc(a), to_lc(b), to_lc(c))); ++n_constraints;
    }
    size_t mul(const Lin &a, const Lin &b) {           // fresh v, enforce a*b = v
        size_t v = alloc(eval(a) * eval(b));
        constrain(a, b, Lin::var(v));
        return v;
    }
    void assert_eq(const Lin &a, const Lin &b) { constrain(a, Lin::cst(FieldT::one()), b); }
    size_t materialize(const Lin &a) { size_t v = alloc(eval(a)); assert_eq(Lin::var(v), a); return v; }

    // ---- Poseidon2 gadget (mirrors HorizenLabs/Plonky3 permutation exactly) ----
    Lin sbox(const Lin &x) {                            // x^5: 3 constraints
        size_t x2 = mul(x, x);
        size_t x4 = mul(Lin::var(x2), Lin::var(x2));
        return Lin::var(mul(Lin::var(x4), x));
    }
    static void mat_external(std::array<Lin, T> &s) {  // circ(2,1,1)
        Lin sum = s[0] + s[1] + s[2];
        for (auto &x : s) x += sum;
    }
    static void mat_internal(std::array<Lin, T> &s) {  // 1 + diag(1,1,2)
        Lin sum = s[0] + s[1] + s[2];
        s[0] += sum; s[1] += sum; s[2] = s[2] * FieldT(2) + sum;
    }
    void permute(std::array<Lin, T> &s) {
        mat_external(s);
        for (int r = 0; r < RF / 2; ++r) {
            for (int i = 0; i < T; ++i) s[i] = sbox(s[i] + Lin::cst(RC[r][i]));
            mat_external(s);
        }
        for (int r = RF / 2; r < RF / 2 + RP; ++r) {
            s[0] = sbox(s[0] + Lin::cst(RC[r][0]));
            mat_internal(s);
        }
        for (int r = RF / 2 + RP; r < NROWS; ++r) {
            for (int i = 0; i < T; ++i) s[i] = sbox(s[i] + Lin::cst(RC[r][i]));
            mat_external(s);
        }
    }
    // Sponge hash; `tag` is a linear combination so it may depend on witness bits
    // (used for the data-dependent chain position). Returns the output variable.
    size_t hash(const std::vector<Lin> &ins, const Lin &tag) {
        std::array<Lin, T> s{ Lin(), Lin(), tag };
        for (size_t k = 0; k < ins.size(); k += RATE) {
            for (int j = 0; j < RATE && k + j < ins.size(); ++j) s[j] += ins[k + j];
            permute(s);
        }
        return materialize(s[0]);
    }

    // Strict decomposition of h into FBITS bits: booleanity, recomposition, and an
    // alias check that the bits encode an integer < p (otherwise h and h+p would
    // both be valid decompositions for small h, letting a prover pick the chunks).
    // `forced` (testing only) supplies the bit pattern instead of h's canonical one.
    std::array<size_t, FBITS> bits_strict(size_t h, const libff::bigint<4> *forced = nullptr) {
        const libff::bigint<4> hv = forced ? *forced : eval(Lin::var(h)).as_bigint();
        std::array<size_t, FBITS> b;
        Lin recomp;
        FieldT pw = FieldT::one();
        for (int j = 0; j < FBITS; ++j) {
            b[j] = alloc(hv.test_bit(j) ? FieldT::one() : FieldT::zero());
            constrain(Lin::var(b[j]), Lin::var(b[j]), Lin::var(b[j]));   // b*b = b
            recomp += Lin::var(b[j], pw);
            pw += pw;
        }
        assert_eq(recomp, Lin::var(h));
        // MSB-first comparison against the constant p. `eq` = "prefix equals p's prefix".
        //   p_j = 1: eq <- eq * b_j        p_j = 0: enforce eq * b_j = 0 (else bits > p)
        // At the end enforce eq = 0 (bits != p). Together: bits < p.
        Lin eq = Lin::cst(FieldT::one());
        for (int j = FBITS - 1; j >= 0; --j) {
            if (FieldT::mod.test_bit(j)) eq = Lin::var(mul(eq, Lin::var(b[j])));
            else constrain(eq, Lin::var(b[j]), Lin());
        }
        assert_eq(eq, Lin());
        return b;
    }
};

// ---------------------- one signer's verification circuit ------------------
// Public (already allocated): msg[MSG_ELEMS], param_s, root_s; epoch is a public constant.
static size_t add_signer_verification(
    Builder &B, uint64_t epoch,
    const std::vector<size_t> &msg_vars, size_t param_var, size_t root_var,
    const FieldT &rho_val, const std::array<FieldT, DIM> &revealed,
    const std::array<FieldT, HGT> &path)
{
    const Lin P = Lin::var(param_var);

    // private inputs
    size_t rho = B.alloc(rho_val);
    std::array<size_t, DIM> start;
    for (int i = 0; i < DIM; ++i) start[i] = B.alloc(revealed[i]);
    std::array<size_t, HGT> sib;
    for (int l = 0; l < HGT; ++l) sib[l] = B.alloc(path[l]);

    // ---- stage 1: encode -> chunk bits ----
    std::vector<Lin> enc_in{ P, Lin::var(rho) };
    for (size_t m : msg_vars) enc_in.push_back(Lin::var(m));
    size_t Hx = B.hash(enc_in, Lin::cst(from_u64(p2_tag_enc(epoch, enc_in.size()))));
    std::array<size_t, FBITS> hbits = B.bits_strict(Hx);

    // ---- stage 2: chains ----
    std::array<size_t, DIM> chain_end;
    for (int i = 0; i < DIM; ++i) {
        std::array<size_t, W_BITS> cb;
        Lin xi;                                         // x_i as a linear combination of its bits
        for (int t = 0; t < W_BITS; ++t) {
            cb[t] = hbits[i * W_BITS + t];
            xi += Lin::var(cb[t], from_u64(1ULL << t));
        }
        // Unrolled walk from the revealed value (at position x_i). Step j produces the
        // value at position x_i + j; the tweak carries that absolute position, which is
        // linear in the chunk bits, so a data-dependent tweak costs nothing.
        std::array<size_t, MAXSTEPS + 1> node;
        node[0] = start[i];
        for (int j = 1; j <= MAXSTEPS; ++j) {
            Lin tag = Lin::cst(from_u64(p2_tag_chain(epoch, i, j)))
                    + xi;                               // + x_i in the position field (low bits)
            node[j] = B.hash({ P, Lin::var(node[j - 1]) }, tag);
        }
        // chain_end = node[MAXSTEPS - x_i] = sum_v [x_i == v] * node[MAXSTEPS - v]
        Lin acc_end;
        for (int v = 0; v < BASE; ++v) {
            Lin sel = Lin::cst(FieldT::one());          // one-hot selector [x_i == v]
            bool first = true;
            for (int t = 0; t < W_BITS; ++t) {
                Lin f = ((v >> t) & 1) ? Lin::var(cb[t]) : Lin::cst(FieldT::one()) - Lin::var(cb[t]);
                if (first) { sel = f; first = false; }
                else sel = Lin::var(B.mul(sel, f));
            }
            acc_end += Lin::var(B.mul(sel, Lin::var(node[MAXSTEPS - v])));
        }
        chain_end[i] = B.materialize(acc_end);
    }

    // ---- stage 3: leaf + Merkle path ----
    std::vector<Lin> leaf_in{ P };
    for (int i = 0; i < DIM; ++i) leaf_in.push_back(Lin::var(chain_end[i]));
    size_t cur = B.hash(leaf_in, Lin::cst(from_u64(p2_tag_leaf(epoch, leaf_in.size()))));
    for (int l = 0; l < HGT; ++l) {
        bool right = (epoch >> l) & 1;                  // public -> order fixed at build time
        std::vector<Lin> in = right ? std::vector<Lin>{ P, Lin::var(sib[l]), Lin::var(cur) }
                                    : std::vector<Lin>{ P, Lin::var(cur), Lin::var(sib[l]) };
        cur = B.hash(in, Lin::cst(from_u64(p2_tag_node(l + 1, epoch >> (l + 1)))));
    }
    B.assert_eq(Lin::var(cur), Lin::var(root_var));
    return start[0];                                    // for the tamper check
}

// Native honest root, mirroring add_signer_verification with the Plonky3 hash.
static FieldT native_root(uint64_t epoch, const std::vector<FieldT> &msg, const FieldT &param,
                          const FieldT &rho, const std::array<FieldT, DIM> &revealed,
                          const std::array<FieldT, HGT> &path)
{
    std::vector<FieldT> enc_in{ param, rho };
    enc_in.insert(enc_in.end(), msg.begin(), msg.end());
    const libff::bigint<4> hx = nhash(enc_in, from_u64(p2_tag_enc(epoch, enc_in.size()))).as_bigint();

    std::vector<FieldT> leaf_in{ param };
    for (int i = 0; i < DIM; ++i) {
        uint64_t xi = 0;
        for (int t = 0; t < W_BITS; ++t) xi |= uint64_t(hx.test_bit(i * W_BITS + t)) << t;
        FieldT v = revealed[i];
        for (uint64_t pos = xi + 1; pos <= MAXSTEPS; ++pos)   // walk x_i -> BASE-1
            v = nhash({ param, v }, from_u64(p2_tag_chain(epoch, i, pos)));
        leaf_in.push_back(v);
    }
    FieldT cur = nhash(leaf_in, from_u64(p2_tag_leaf(epoch, leaf_in.size())));
    for (int l = 0; l < HGT; ++l) {
        bool right = (epoch >> l) & 1;
        FieldT tag = from_u64(p2_tag_node(l + 1, epoch >> (l + 1)));
        cur = right ? nhash({ param, path[l], cur }, tag) : nhash({ param, cur, path[l] }, tag);
    }
    return cur;
}

// ------------------------------- self tests --------------------------------
static bool selftest_gadget_vs_plonky3() {
    for (int trial = 0; trial < 4; ++trial) {
        std::array<FieldT, T> in{ FieldT::random_element(), FieldT::random_element(), FieldT::random_element() };
        Builder B;
        std::array<Lin, T> s;
        for (int i = 0; i < T; ++i) s[i] = Lin::var(B.alloc_pub(in[i]));
        B.permute(s);
        std::array<FieldT, T> ref = in;
        p3_permute(ref);
        for (int i = 0; i < T; ++i) if (B.eval(s[i]) != ref[i]) return false;
        if (trial == 0) printf("[aurora-verify] Poseidon2 permutation gadget: %zu constraints\n", B.n_constraints);
    }
    return true;
}

static bool satisfied(Builder &B) {
    B.cs.primary_input_size_ = B.num_pub;
    B.cs.auxiliary_input_size_ = B.asg.size() - B.num_pub;
    libiop::r1cs_primary_input<FieldT>   pi(B.asg.begin(), B.asg.begin() + B.num_pub);
    libiop::r1cs_auxiliary_input<FieldT> ai(B.asg.begin() + B.num_pub, B.asg.end());
    return B.cs.is_satisfied(pi, ai);
}

// The alias check must reject the decomposition of h + p (same value mod p).
static bool selftest_alias_check() {
    Builder ok, bad;
    size_t h1 = ok.alloc_pub(from_u64(5));  ok.bits_strict(h1);
    size_t h2 = bad.alloc_pub(from_u64(5));
    libff::bigint<4> alias = FieldT::mod;   // p + 5 < 2^254
    mpn_add_1(alias.data, alias.data, 4, 5);
    bad.bits_strict(h2, &alias);
    return satisfied(ok) && !satisfied(bad);
}

static size_t next_pow2(size_t n) { size_t p = 1; while (p < n) p <<= 1; return p; }

// ------------------------- instance file (export_p2_sigs) -------------------------
struct Signer { FieldT param, root, rho; std::array<FieldT, DIM> revealed; std::array<FieldT, HGT> path;
                size_t param_var = 0, root_var = 0; };
struct Instance { uint64_t epoch = 0; std::vector<FieldT> msg; std::vector<Signer> signers; };

static FieldT from_hex(const std::string &h) {
    if (h.size() != 64) throw std::runtime_error("instance: bad field element '" + h + "'");
    uint64_t l[4];
    for (int i = 0; i < 4; ++i) l[3 - i] = std::stoull(h.substr(16 * i, 16), nullptr, 16);
    return from_limbs(l);
}

static Instance load_instance(const char *path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error(std::string("cannot open ") + path);
    std::string line, word;
    Instance I;
    size_t k = 0;
    while (std::getline(in, line)) {
        std::istringstream ls(line);
        ls >> word;
        if (word == "params") {
            int w, d, h, m; ls >> w >> d >> h >> m;
            if (w != W_BITS || d != DIM || h != HGT || m != MSG_ELEMS)
                throw std::runtime_error("instance params do not match this circuit build");
        } else if (word == "k") { ls >> k; }
        else if (word == "epoch") { ls >> I.epoch; }
        else if (word == "msg") { while (ls >> word) I.msg.push_back(from_hex(word)); }
        else if (word == "signer") {
            Signer s;
            ls >> word; s.param = from_hex(word);
            ls >> word; s.root = from_hex(word);
            ls >> word; s.rho = from_hex(word);
            for (auto &x : s.revealed) { ls >> word; x = from_hex(word); }
            for (auto &x : s.path)     { ls >> word; x = from_hex(word); }
            I.signers.push_back(s);
        }
    }
    if (I.signers.size() != k || k == 0 || I.msg.size() != MSG_ELEMS)
        throw std::runtime_error("instance file is incomplete");
    return I;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <instance-file> [zk=1|0]\n", argv[0]); return 2; }
    const bool make_zk = argc < 3 || std::atoi(argv[2]) != 0;

    libff::alt_bn128_pp::init_public_params();
    load_poseidon2_from_plonky3();

    if (!selftest_gadget_vs_plonky3()) { printf("[aurora-verify] ERROR: gadget != Plonky3 Poseidon2\n"); return 1; }
    printf("[aurora-verify] self-test: R1CS Poseidon2 gadget == Plonky3 Poseidon2Bn254<3> (via FFI)\n");
    if (!selftest_alias_check()) { printf("[aurora-verify] ERROR: alias check broken\n"); return 1; }
    printf("[aurora-verify] self-test: strict bit decomposition rejects the h+p alias\n");

    Instance I = load_instance(argv[1]);
    const size_t K = I.signers.size();
    const uint64_t epoch = I.epoch;

    // The core signed with Poseidon2TweakHash; recompute each root with the circuit's
    // hash schedule. Equality = the core and the circuit hash identically.
    for (size_t s = 0; s < K; ++s) {
        const Signer &sg = I.signers[s];
        if (native_root(epoch, I.msg, sg.param, sg.rho, sg.revealed, sg.path) != sg.root) {
            printf("[aurora-verify] ERROR: signer %zu: core signature does not verify under the circuit's hash\n", s);
            return 1;
        }
    }
    printf("[aurora-verify] loaded %zu REAL core signatures (epoch %lu); native roots == public keys\n",
           K, (unsigned long)epoch);

    Builder B;
    std::vector<size_t> msg_vars(MSG_ELEMS);
    for (int i = 0; i < MSG_ELEMS; ++i) msg_vars[i] = B.alloc_pub(I.msg[i]);
    B.alloc_pub(from_u64(epoch));                       // bind epoch into the statement
    for (auto &s : I.signers) {
        s.param_var = B.alloc_pub(s.param);
        s.root_var  = B.alloc_pub(s.root);
    }
    size_t target_inputs = next_pow2(B.num_pub + 1) - 1;  // Aurora: (#inputs + 1) is a power of 2
    while (B.num_pub < target_inputs) B.alloc_pub(FieldT::one());

    auto tb0 = std::chrono::high_resolution_clock::now();
    size_t tamper_var = 0;
    for (size_t s = 0; s < K; ++s) {
        const Signer &sg = I.signers[s];
        size_t v = add_signer_verification(B, epoch, msg_vars, sg.param_var, sg.root_var,
                                           sg.rho, sg.revealed, sg.path);
        if (s == 0) tamper_var = v;
    }
    const size_t real_constraints = B.n_constraints;

    // pad variables/constraints to Aurora-friendly sizes
    size_t target_vars = next_pow2(B.asg.size() + 1) - 1;
    while (B.asg.size() < target_vars) B.asg.push_back(FieldT::one());
    size_t target_constraints = next_pow2(std::max(B.n_constraints, B.asg.size()));
    while (B.n_constraints < target_constraints) B.constrain(Lin(), Lin(), Lin());   // 0*0=0
    auto tb1 = std::chrono::high_resolution_clock::now();

    B.cs.primary_input_size_   = B.num_pub;
    B.cs.auxiliary_input_size_ = B.asg.size() - B.num_pub;
    libiop::r1cs_primary_input<FieldT>   primary(B.asg.begin(), B.asg.begin() + B.num_pub);
    libiop::r1cs_auxiliary_input<FieldT> auxiliary(B.asg.begin() + B.num_pub, B.asg.end());

    printf("[aurora-verify] k=%zu signers | DIM=%d chains, BASE=%d, HGT=%d | hash = Poseidon2-BN254 (t=3, x^5, 8+56 rounds)\n",
           K, DIM, BASE, HGT);
    printf("[aurora-verify] R1CS: %zu real constraints (%zu/signer), padded to %zu; %zu variables, %zu public inputs\n",
           real_constraints, real_constraints / K, B.cs.num_constraints(), B.cs.num_variables(), B.cs.num_inputs());

    if (!B.cs.is_satisfied(primary, auxiliary)) {
        printf("[aurora-verify] ERROR: R1CS NOT satisfied (gadget/native mismatch)\n");
        return 1;
    }
    printf("[aurora-verify] R1CS satisfied by the real signatures.\n");
    {
        libiop::r1cs_auxiliary_input<FieldT> bad = auxiliary;
        bad[tamper_var - 1 - B.num_pub] += FieldT::one();   // signer 0's first revealed chain value
        printf("[aurora-verify] tamper check (corrupt a revealed chain value): R1CS %s (expected rejected)\n",
               B.cs.is_satisfied(primary, bad) ? "STILL ACCEPTED - BUG!" : "rejected");
    }

    // ---- prove + verify with Aurora (multiplicative coset domain for a prime field) ----
    typedef libiop::binary_hash_digest hash_type;
    const size_t security_parameter = 128, RS_extra_dimensions = 2, FRI_localization_parameter = 3;
    libiop::aurora_snark_parameters<FieldT, hash_type> params(
        security_parameter,
        libiop::LDT_reducer_soundness_type::optimistic_heuristic,
        libiop::FRI_soundness_type::heuristic,
        libiop::blake2b_type, FRI_localization_parameter, RS_extra_dimensions,
        make_zk, libiop::multiplicative_coset_type,
        B.cs.num_constraints(), B.cs.num_variables());

    auto t0 = std::chrono::high_resolution_clock::now();
    const auto argument = libiop::aurora_snark_prover<FieldT>(B.cs, primary, auxiliary, params);
    auto t1 = std::chrono::high_resolution_clock::now();
    const long build_ms  = (long)std::chrono::duration_cast<std::chrono::milliseconds>(tb1 - tb0).count();
    const long prove_ms  = (long)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    printf("[aurora-verify] proof size: %lu bytes | prove %ld ms (zk=%d)\n", argument.size_in_bytes(), prove_ms, make_zk);

    auto t2 = std::chrono::high_resolution_clock::now();
    const bool ok = libiop::aurora_snark_verifier<FieldT, hash_type>(B.cs, primary, argument, params);
    auto t3 = std::chrono::high_resolution_clock::now();
    const long verify_ms = (long)std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    printf("[aurora-verify] verify %ld ms | VERIFICATION: %s\n", verify_ms, ok ? "SUCCESS" : "FAILED");
    printf("RESULT system=aurora k=%zu zk=%d constraints=%zu padded=%zu proof_bytes=%lu build_ms=%ld prove_ms=%ld verify_ms=%ld ok=%d\n",
           K, make_zk, real_constraints, B.cs.num_constraints(), argument.size_in_bytes(),
           build_ms, prove_ms, verify_ms, ok);
    return ok ? 0 : 1;
}
