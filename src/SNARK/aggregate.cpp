#include "myR1CS.hpp"

#include <libff/algebra/fields/binary/gf64.hpp>
#include <libff/algebra/curves/edwards/edwards_pp.hpp>
#include <libiop/snark/aurora_snark.hpp>

namespace libiop
{
    using namespace std;

<<<<<<< HEAD
    using stmnt_T = tuple<size_t, size_t, vector<uint8_t>, vector<vector<uint8_t>>>;
    using witn_T = vector<vector<uint8_t>>;
=======
using stmnt_T = tuple<size_t, size_t, vector<uint8_t>, vector<vector<uint8_t>>>;
using witn_T = vector<vector<uint8_t>>;
>>>>>>> 7c270f1ab0c8c252d3090eec90da0c5b4d6f607c

    void aggregate(stmnt_T statement, witn_T witness)
    {
        typedef libff::gf64 FieldT;
        typedef binary_hash_digest hash_type;

<<<<<<< HEAD
        aggregate_r1cs<FieldT> r1cs = generate_aggregate_r1cs<FieldT>(statement, witness);

        const size_t num_constraints = r1cs.num_constraints();
        const size_t num_inputs = r1cs.num_inputs();
        const size_t num_variables = r1cs.num_variables();
        const size_t security_parameter = 128;
        const size_t RS_extra_dimensions = 2;
        const size_t FRI_localization_parameter = 3;
        const LDT_reducer_soundness_type ldt_reducer_soundness_type = LDT_reducer_soundness_type::optimistic_heuristic;
        const FRI_soundness_type fri_soundness_type = FRI_soundness_type::heuristic;
        const field_subset_type domain_type = affine_subspace_type;

        aurora_snark_parameters<FieldT, hash_type> params(
=======
    aggregate_r1cs<FieldT> r1cs = generate_aggregate_r1cs<FieldT>(statement, witness);

    const size_t num_constraints = r1cs.num_constraints();
    const size_t num_inputs = r1cs.num_inputs();
    const size_t num_variables = r1cs.num_variables();
    const size_t security_parameter = 128;
    const size_t RS_extra_dimensions = 2;
    const size_t FRI_localization_parameter = 3;
    const LDT_reducer_soundness_type ldt_reducer_soundness_type = LDT_reducer_soundness_type::optimistic_heuristic;
    const FRI_soundness_type fri_soundness_type = FRI_soundness_type::heuristic;
    const field_subset_type domain_type = affine_subspace_type;

    aurora_snark_parameters<FieldT, hash_type> params(
>>>>>>> 7c270f1ab0c8c252d3090eec90da0c5b4d6f607c
            security_parameter,
            ldt_reducer_soundness_type,
            fri_soundness_type,
            blake2b_type,
            FRI_localization_parameter,
            RS_extra_dimensions,
            1,
            domain_type,
            num_constraints,
            num_variables);

        const aurora_snark_argument<FieldT, hash_type> argument = aurora_snark_prover<FieldT>(
            r1cs.constraint_system_,
            r1cs.primary_input_,
            r1cs.auxiliary_input_,
            params);

        printf("iop size in bytes %lu\n", argument.IOP_size_in_bytes());
        printf("bcs size in bytes %lu\n", argument.BCS_size_in_bytes());
        printf("argument size in bytes %lu\n", argument.size_in_bytes());

        const bool bit = aurora_snark_verifier<FieldT, hash_type>(
            r1cs.constraint_system_,
            r1cs.primary_input_,
            argument,
            params);
    }

};