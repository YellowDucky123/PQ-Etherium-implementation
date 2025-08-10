#ifndef MYR1CS_HPP_
#define MYR1CS_HPP_

#include <libiop/relations/r1cs.hpp>

namespace libiop {

using stmnt_T = std::tuple<std::size_t, std::size_t, std::vector<uint8_t>, std::vector<std::vector<uint8_t>>>;
using witn_T = std::vector<std::vector<uint8_t>>;

template<typename FieldT>
struct aggregate_r1cs {
    r1cs_constraint_system<FieldT> constraint_system_;
    r1cs_primary_input<FieldT> primary_input_;
    r1cs_auxiliary_input<FieldT> auxiliary_input_;

    aggregate_r1cs<FieldT>(const r1cs_constraint_system<FieldT> &constraint_system,
                         const r1cs_primary_input<FieldT> &primary_input,
                         const r1cs_auxiliary_input<FieldT> &auxiliary_input) :
        constraint_system_(constraint_system),
        primary_input_(primary_input),
        auxiliary_input_(auxiliary_input)
    {};
    aggregate_r1cs<FieldT>(r1cs_constraint_system<FieldT> &&constraint_system,
                         r1cs_primary_input<FieldT> &&primary_input,
                         r1cs_auxiliary_input<FieldT> &&auxiliary_input) :
        constraint_system_(std::move(constraint_system)),
        primary_input_(std::move(primary_input)),
        auxiliary_input_(std::move(auxiliary_input))
    {};

    std::size_t num_inputs() const {
        return constraint_system_.num_inputs();
    }
    std::size_t num_variables() const {
        return constraint_system_.num_variables();
    }
    std::size_t num_constraints() const {
        return constraint_system_.num_constraints();
    }
};

template<typename FieldT>
aggregate_r1cs<FieldT> generate_aggregate_r1cs(stmnt_T statement, witn_T witness);

};

#include "libiop/relations/examples/myR1CS.tcc"

#endif // MYR1CS_HPP_