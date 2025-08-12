/* 
This File is supposed to be the R1CS for aggregating the Multi Signatures with Aurora.
However, due to deadlines this remains unfinished
*/


#include <cassert>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <tuple>

#include <libiop/algebra/utils.hpp>

#include "../signature/generalized_xmss.hpp"

namespace libiop {

// PublicKey and Signature type cannot be used like this, Implementation still unsure, possible OOP design refactor required 
using stmnt_T = std::tuple<std::size_t, std::size_t, std::vector<uint8_t>, std::vector<PublicKey>>;
using witn_T = std::vector<Signature>;

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

    std::vector<PublicKey> PKs = std::get<3>(statement);
    for(const auto &pk : PKs) {
        for(const auto &bit : pk) {
            primary_input.emplace_back(FieldT(bit));
        }
    }

    /* The auxilary input (the witnesses but flattened into FieldT's) */
    r1cs_auxiliary_input<FieldT> auxiliary_input;
    for(const auto &signature : witness) {
        for(const auto &bit : signature) {
            auxiliary_input.emplace_back(FieldT(bit));
        }
    }

    cs.primary_input_size_ = primary_input.size();
    cs.auxiliary_input_size_ = auxiliary_input.size();

    /* constraint for all signature witness have to be verified */
    for(int i = 0; i < nonField_k; i++) {
        linear_combination<FieldT> A, B, C;
        /* Verify function still needs to be implemented in generalized_xmss.hpp
           Verify would need to go through all stored SignatureVerificationFunctions until either approved or
           not approved by any
        */
        int ver_s = MultiSignatureVerification::verify(PKs[i], ep, m, witness[i]); 
        A.add_term(ver_s - 1);
        B.add_term(1);
        C.add_term(0);

        cs.add_constraint(r1cs_constraint<FieldT>(A, B, C));
    }



    assert(cs.is_satisfied(primary_input, auxiliary_input) && "constraint system is not satisfied");

    // Construct and return an aggregate_r1cs object
    return aggregate_r1cs<FieldT>(std::move(cs), std::move(pi), std::move(ai));
}
};
