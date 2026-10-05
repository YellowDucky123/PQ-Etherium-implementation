#pragma once

#include "../TweakHash.hpp"
#include "../poseidon2_bn254.hpp"
#include "poseidon2_tags.h"
#include <vector>
#include <cstdint>
#include <memory>

/// Tweakable hash over Poseidon2-BN254 (Plonky3's Poseidon2Bn254<3>).
///
/// apply(param, tweak, msgs) = Sponge_tag(param, msgs...), where the tweak is encoded
/// as a u64 tag in the capacity element (see poseidon2_tags.h). Parameter and Domain are
/// single field elements, stored as 32-byte little-endian strings.
///
/// This is the instance the Aurora R1CS (SNARK/aurora/aggregate_aurora_verify.cpp) and the
/// Plonky3 AIR (SNARK/plonky3_air) arithmetize, so signatures produced with it can be
/// aggregated by either prover.
struct Poseidon2Tweak {
    enum Kind { TREE, CHAIN } kind;
    uint32_t a, b, c;   // TREE: (level, pos_in_level, -)   CHAIN: (epoch, chain_index, pos_in_chain)

    uint64_t tag(size_t num_absorbed) const {
        if (kind == CHAIN) return p2_tag_chain(a, b, c);
        if (a == 0) return p2_tag_leaf(b, num_absorbed);
        return p2_tag_node(a, b);
    }
};

struct Poseidon2TweakHash : public TweakableHash<std::vector<uint8_t>, Poseidon2Tweak, std::vector<uint8_t>>
{
    Parameter rand_parameter() override { return p2bn254::rand_bytes(); }

    Domain rand_domain() override { return p2bn254::rand_bytes(); }

    std::unique_ptr<Poseidon2Tweak> tree_tweak(uint8_t level, uint32_t pos_in_level) override {
        return std::make_unique<Poseidon2Tweak>(Poseidon2Tweak{Poseidon2Tweak::TREE, level, pos_in_level, 0});
    }

    std::unique_ptr<Poseidon2Tweak> chain_tweak(uint32_t epoch, uint8_t chain_index, uint8_t pos_in_chain) override {
        return std::make_unique<Poseidon2Tweak>(Poseidon2Tweak{Poseidon2Tweak::CHAIN, epoch, chain_index, pos_in_chain});
    }

    Domain apply(Parameter parameter, Poseidon2Tweak &tweak, std::vector<Domain> &messages) override {
        std::vector<p2bn254::Fe> ins;
        ins.reserve(1 + messages.size());
        ins.push_back(p2bn254::from_bytes(parameter));
        for (const Domain &m : messages) ins.push_back(p2bn254::from_bytes(m));
        return p2bn254::to_bytes(p2bn254::sponge(ins, tweak.tag(ins.size())));
    }

    void internal_consistency_check() override {}
};
