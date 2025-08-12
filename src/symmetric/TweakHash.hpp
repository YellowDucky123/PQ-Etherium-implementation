#pragma once

#include <openssl/rand.h>
#include <cstdint>
#include <memory>

template <typename Parameter_i, typename Tweak_i, typename Domain_i>
struct TweakableHash {
    typedef Parameter_i Parameter;
    typedef Tweak_i Tweak;
    typedef Domain_i Domain;

    using Parameter = Parameter_i;
    using Tweak = Tweak_i;
    using Domain = Domain_i;

    virtual Parameter rand_parameter() = 0;

    virtual Domain rand_domain() = 0;

    /// Returns a tweak to be used in the Merkle tree.
    /// Note: this is assumed to be distinct from the outputs of chain_tweak
    virtual std::unique_ptr<Tweak> tree_tweak(uint8_t level, uint32_t pos_in_level) = 0;

    /// Returns a tweak to be used in chains.
    /// Note: this is assumed to be distinct from the outputs of tree_tweak
    virtual std::unique_ptr<Tweak> chain_tweak(uint32_t epoch, uint8_t chain_index, uint8_t pos_in_chain) = 0;

    /// Applies the tweakable hash to parameter, tweak, and message.
    virtual Domain apply(Parameter parameter, Tweak& tweak, Domain&) = 0;

    virtual void internal_consistency_check() = 0;
};

template <typename TH>
typename TH::Domain chain(TH &th, typename TH::Parameter &parameter,
     uint32_t epoch, uint8_t chain_index, uint8_t start_pos_in_chain, uint steps, typename TH::Domain &start) {
    using TH_domain = typename TH::Domain;
    
    TH_domain current = *start;

    for(uint j = 0; j < steps; j++) {
        TH::Tweak tweak = th->chain_tweak(epoch, chain_index, start_pos_in_chain + static_cast<uint8_t>(j) + static_cast<uint8_t>(1));
        current = th->apply(parameter, &tweak, current.data());
    }

    return current;
}