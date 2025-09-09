#ifndef MYR1CS_HPP_
#define MYR1CS_HPP_

#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <tuple>

#include <libiop/relations/r1cs.hpp>
#include "../signature/generalized_xmss.hpp"
#include "Variants.hpp"

namespace libiop {
using namespace std;

using stmnt_T = tuple<size_t, size_t, vector<uint8_t>, vector<pk_variants>>;
using witn_T = vector<sig_variants>;

template<typename FieldT>
struct aggregate_r1cs {
    r1cs_constraint_system<FieldT> constraint_system_;
    r1cs_primary_input<FieldT> primary_input_;
    r1cs_auxiliary_input<FieldT> auxiliary_input_;

    aggregate_r1cs(const r1cs_constraint_system<FieldT> &constraint_system,
                         const r1cs_primary_input<FieldT> &primary_input,
                         const r1cs_auxiliary_input<FieldT> &auxiliary_input) :
        constraint_system_(constraint_system),
        primary_input_(primary_input),
        auxiliary_input_(auxiliary_input)
    {};
    aggregate_r1cs(const r1cs_constraint_system<FieldT> &&constraint_system,
                         const r1cs_primary_input<FieldT> &&primary_input,
                         const r1cs_auxiliary_input<FieldT> &&auxiliary_input) :
        constraint_system_(move(constraint_system)),
        primary_input_(move(primary_input)),
        auxiliary_input_(move(auxiliary_input))
    {};

    size_t num_inputs() const {
        return constraint_system_.num_inputs();
    }
    size_t num_variables() const {
        return constraint_system_.num_variables();
    }
    size_t num_constraints() const {
        return constraint_system_.num_constraints();
    }
};

template<typename FieldT>
aggregate_r1cs<FieldT> generate_aggregate_r1cs(stmnt_T statement, witn_T witness) {
    r1cs_constraint_system<FieldT> cs;
    r1cs_primary_input<FieldT> pi; // just std::vector<FieldT>
    r1cs_auxiliary_input<FieldT> ai; // just std::vector<FieldT>


    /* The primary_input (the statements but flattened into FieldT's)  */
    r1cs_primary_input<FieldT> primary_input;
    std::size_t nonField_k = std::get<0>(statement);
    FieldT k = FieldT(std::get<0>(statement));
    FieldT ep = FieldT(std::get<1>(statement));

    primary_input.emplace_back(k);
    primary_input.emplace_back(ep);

    std::vector<uint8_t> m = std::get<2>(statement);
    for(const auto &bit : m) {
        primary_input.emplace_back(FieldT(bit));
    }

    std::vector<pk_variants> PKs = std::get<3>(statement);
    for(const auto &pk : PKs) {
        for(const auto &bit : pk) {
            primary_input.emplace_back(FieldT(bit));
        }
    }

    /* The auxilary input (the witnesses but flattened into FieldT's) */
    r1cs_auxiliary_input<FieldT> auxiliary_input;
    for(const sig_variants &signature : witness) {
        for(const auto &bit : signature) {
            auxiliary_input.emplace_back(FieldT(bit));
        }
    }

    cs.primary_input_size_ = primary_input.size();
    cs.auxiliary_input_size_ = auxiliary_input.size();

    /* constraint for all signature witness have to be verified */
    for(int i = 0; i < nonField_k; i++) {
        linear_combination<FieldT> A, B, C;
        // (const PublicKey& pk, uint32_t epoch, const std::array<uint8_t, MESSAGE_LENGTH>& msg, const Signature& sig) {}
        int ver_s = GeneralizedXMSSSignatureScheme::verify(PKs[i], ep, m, witness[i]); // sig_verify still not sure where it's implemented
        A.add_term(ver_s - 1);
        B.add_term(1);
        C.add_term(0);

        cs.add_constraint(r1cs_constraint<FieldT>(A, B, C));
    }

    assert(cs.is_satisfied(primary_input, auxiliary_input) && "constraint system is not satisfied");

    // Construct and return an aggregate_r1cs object
    return aggregate_r1cs<FieldT>(std::move(cs), std::move(pi), std::move(ai));
}

std::vector<FieldT> signature_flatten(const sig_variants &signature) {
    
}

};

#endif // MYR1CS_HPP_