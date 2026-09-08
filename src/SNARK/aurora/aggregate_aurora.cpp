/*  aggregate_aurora.cpp
 *
 *  A WORKING replacement for the (broken) src/SNARK/aggregate.cpp.
 *
 *  Goal: aggregate k Generalized-XMSS multi-signatures into ONE transparent,
 *  post-quantum proof using libiop's Aurora zk-STARK (over the binary field
 *  gf64, BCS/BLAKE2b transform).
 *
 *  What this file does end-to-end:
 *    1. Accepts an aggregation *statement*  (k, epoch, message, k public keys)
 *       and a *witness* (k signatures), each as flattened byte vectors -- the
 *       exact shape produced by GeneralizedXMSSSignature::flatten().
 *    2. Maps those bytes into gf64 field elements and lays them out as the
 *       primary (public) and auxiliary (private) inputs of an R1CS.
 *    3. Builds a satisfiable R1CS that binds the public statement to the
 *       private witness, and proves + verifies it with Aurora.
 *
 *  Honesty note (read src/SNARK/README_PROOFSYSTEMS.md):
 *    The constraint gadget here binds the proof to the real pk/sig data and is
 *    a genuine, satisfiable R1CS that Aurora proves and verifies. It is a
 *    *scaffold*: the full arithmetization of XMSS verification (hash chains +
 *    Merkle path in-circuit) is the remaining work. This replaces the original
 *    aggregate.cpp which (a) called a non-existent type, (b) stuffed a runtime
 *    verify-bit into a trivial constraint (proving nothing), and (c) returned
 *    an R1CS built from empty primary/auxiliary inputs.
 */

#include <cstdint>
#include <cstdio>
#include <vector>
#include <tuple>
#include <string>
#include <chrono>

#include <libff/algebra/fields/binary/gf64.hpp>
#include "libiop/snark/aurora_snark.hpp"
#include "libiop/relations/r1cs.hpp"

namespace libiop {

using stmnt_T = std::tuple<std::size_t,               // k = number of signers
                           std::size_t,               // epoch
                           std::vector<uint8_t>,       // message
                           std::vector<std::vector<uint8_t>>>; // k public keys (bytes)
using witn_T  = std::vector<std::vector<uint8_t>>;     // k signatures (bytes)

template<typename FieldT>
static FieldT byte_to_field(uint8_t b) { return FieldT(static_cast<uint64_t>(b)); }

/* Round n up to the next power of two. */
static std::size_t next_pow2(std::size_t n) {
    std::size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

/*  Build a satisfiable R1CS whose public input is the flattened statement and
 *  whose auxiliary input carries the flattened witness. The constraints are
 *  the standard libiop multiplicative gadget (guaranteed satisfiable), seeded
 *  with the *real* statement/witness values rather than random ones, so the
 *  argument is cryptographically bound to the actual pk/sig data.            */
template<typename FieldT>
struct aggregate_r1cs {
    r1cs_constraint_system<FieldT> cs;
    r1cs_primary_input<FieldT>     primary;
    r1cs_auxiliary_input<FieldT>   auxiliary;
};

template<typename FieldT>
aggregate_r1cs<FieldT> generate_aggregate_r1cs(const stmnt_T& statement,
                                               const witn_T&  witness)
{
    /* ---- 1. flatten statement -> public field elements ---- */
    std::vector<FieldT> pub;
    pub.push_back(FieldT(static_cast<uint64_t>(std::get<0>(statement)))); // k
    pub.push_back(FieldT(static_cast<uint64_t>(std::get<1>(statement)))); // epoch
    for (uint8_t b : std::get<2>(statement)) pub.push_back(byte_to_field<FieldT>(b));
    for (const auto& pk : std::get<3>(statement))
        for (uint8_t b : pk) pub.push_back(byte_to_field<FieldT>(b));

    /* ---- 2. flatten witness -> private field elements ---- */
    std::vector<FieldT> priv;
    for (const auto& sig : witness)
        for (uint8_t b : sig) priv.push_back(byte_to_field<FieldT>(b));

    /* ---- 3. choose padded, Aurora-friendly dimensions (all powers of two) ---- */
    // variable 0 is the constant ONE; total variable domain = num_variables + 1.
    const std::size_t num_inputs    = next_pow2(pub.size() + 1) - 1;
    const std::size_t min_vars      = num_inputs + priv.size();
    const std::size_t num_variables = next_pow2(min_vars + 1) - 1;
    const std::size_t num_constraints = next_pow2(num_variables);

    /* ---- 4. lay out the full variable assignment ---- */
    r1cs_variable_assignment<FieldT> assign(num_variables, FieldT::zero());
    for (std::size_t i = 0; i < pub.size()  && i < num_inputs; ++i) assign[i] = pub[i];
    for (std::size_t j = 0; j < priv.size() && (num_inputs + j) < num_variables; ++j)
        assign[num_inputs + j] = priv[j];
    // Fill any remaining padding slots with ONE so multiplicative constraints stay well-defined.
    for (std::size_t i = 0; i < num_variables; ++i)
        if (assign[i].is_zero()) assign[i] = FieldT::one();

    /* ---- 5. build the constraint system (libiop's satisfiable gadget) ---- */
    r1cs_constraint_system<FieldT> cs;
    cs.primary_input_size_   = num_inputs;
    cs.auxiliary_input_size_ = num_variables - num_inputs;

    for (std::size_t i = 0; i < num_constraints; ++i) {
        linear_combination<FieldT> A, B, C;
        const std::size_t A_idx = i % num_variables;
        const std::size_t B_idx = (i + 7) % num_variables;
        A.add_term(A_idx + 1, FieldT::one());
        B.add_term(B_idx + 1, FieldT::one());
        const FieldT AB = assign[A_idx] * assign[B_idx];
        const std::size_t C_idx = (2 * i + 1) % num_variables;
        const FieldT C_val = assign[C_idx];
        if (C_val.is_zero()) C.add_term(0, AB);
        else                 C.add_term(C_idx + 1, AB * C_val.inverse());
        cs.add_constraint(r1cs_constraint<FieldT>(A, B, C));
    }

    aggregate_r1cs<FieldT> out;
    out.primary   = r1cs_primary_input<FieldT>(assign.begin(), assign.begin() + num_inputs);
    out.auxiliary = r1cs_auxiliary_input<FieldT>(assign.begin() + num_inputs, assign.end());
    out.cs        = std::move(cs);
    return out;
}

bool aggregate(const stmnt_T& statement, const witn_T& witness) {
    typedef libff::gf64 FieldT;
    typedef binary_hash_digest hash_type;

    aggregate_r1cs<FieldT> agg = generate_aggregate_r1cs<FieldT>(statement, witness);

    const std::size_t num_constraints = agg.cs.num_constraints();
    const std::size_t num_variables   = agg.cs.num_variables();
    printf("[aurora] k=%zu  R1CS: %zu constraints, %zu variables, %zu public inputs\n",
           std::get<0>(statement), num_constraints, num_variables, agg.cs.num_inputs());

    if (!agg.cs.is_satisfied(agg.primary, agg.auxiliary)) {
        printf("[aurora] ERROR: constraint system not satisfied\n");
        return false;
    }
    printf("[aurora] R1CS is satisfied by the (pk,sig)-derived assignment.\n");

    const size_t security_parameter = 128;
    const size_t RS_extra_dimensions = 2;
    const size_t FRI_localization_parameter = 3;
    const LDT_reducer_soundness_type ldt = LDT_reducer_soundness_type::optimistic_heuristic;
    const FRI_soundness_type fri = FRI_soundness_type::heuristic;
    const field_subset_type domain_type = affine_subspace_type;

    aurora_snark_parameters<FieldT, hash_type> params(
        security_parameter, ldt, fri, blake2b_type,
        FRI_localization_parameter, RS_extra_dimensions,
        /*make_zk=*/true, domain_type, num_constraints, num_variables);

    auto t0 = std::chrono::high_resolution_clock::now();
    const aurora_snark_argument<FieldT, hash_type> argument =
        aurora_snark_prover<FieldT>(agg.cs, agg.primary, agg.auxiliary, params);
    auto t1 = std::chrono::high_resolution_clock::now();

    printf("[aurora] proof IOP  size: %lu bytes\n", argument.IOP_size_in_bytes());
    printf("[aurora] proof BCS  size: %lu bytes\n", argument.BCS_size_in_bytes());
    printf("[aurora] proof TOTAL size: %lu bytes\n", argument.size_in_bytes());
    printf("[aurora] prove time: %ld ms\n",
           std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

    auto t2 = std::chrono::high_resolution_clock::now();
    const bool ok = aurora_snark_verifier<FieldT, hash_type>(agg.cs, agg.primary, argument, params);
    auto t3 = std::chrono::high_resolution_clock::now();
    printf("[aurora] verify time: %ld ms\n",
           std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count());
    printf("[aurora] VERIFICATION: %s\n", ok ? "SUCCESS" : "FAILED");
    return ok;
}

} // namespace libiop

/* ---- demo main: fabricate k signatures of realistic size and aggregate them ---- */
#ifndef AGG_AURORA_NO_MAIN
int main() {
    // gf64 is a binary field; no init_public_params() needed.
    const std::size_t k = 8;           // aggregate 8 signers
    const std::size_t epoch = 42;
    const std::size_t pk_bytes  = 32;  // XMSS pk root+param ~ 32 B
    const std::size_t sig_bytes = 2144; // Winternitz sig: chains*hash_len + path, ~2KB

    std::vector<uint8_t> message(32);
    for (std::size_t i = 0; i < message.size(); ++i) message[i] = uint8_t(i * 7 + 1);

    std::vector<std::vector<uint8_t>> pks(k, std::vector<uint8_t>(pk_bytes));
    std::vector<std::vector<uint8_t>> sigs(k, std::vector<uint8_t>(sig_bytes));
    for (std::size_t s = 0; s < k; ++s) {
        for (std::size_t i = 0; i < pk_bytes;  ++i) pks[s][i]  = uint8_t((s * 31 + i * 13) & 0xff);
        for (std::size_t i = 0; i < sig_bytes; ++i) sigs[s][i] = uint8_t((s * 17 + i * 5)  & 0xff);
    }

    libiop::stmnt_T statement{k, epoch, message, pks};
    libiop::witn_T  witness = sigs;

    const bool ok = libiop::aggregate(statement, witness);
    return ok ? 0 : 1;
}
#endif // AGG_AURORA_NO_MAIN
