#pragma once


#include "TweakHash.hpp"
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <openssl/rand.h>
#include <concepts>
#include <assert.h>

template <typename TH>
concept TweakableHash_c = requires(TH t) {
    []<typename X, typename Y, typename Z>(TweakableHash<X, Y, Z>&){}(t);
};

/// A single layer of a sparse Hash-Tree
/// based on tweakable hash function
template <TweakableHash_c TH>
struct HashTreeLayer {
    uint start_index;
    std::vector<typename TH::Domain> nodes;

    HashTreeLayer(uint _start_index, std::vector<typename TH::Domain> _nodes) 
        : start_index(_start_index), nodes(std::move(_nodes)) {}
};

template <TweakableHash_c TH>
struct HashTreeOpening {
    /// The co-path needed to verify
    /// If the tree has depth h, i.e, 2^h leafs
    /// the co-path should have size D
    std::vector<typename TH::Domain> co_path;

    HashTreeOpening(std::vector<typename TH::Domain> _co_path) : co_path(std::move(_co_path)) {}
};

# if defined(_OPENMP) && defined(VERBOSE)
    printf_s("Compiled by an OpenMP-compliant implementation.\n");
# endif

template <TweakableHash_c TH>
class HashTree {
    using TH_parameter = typename TH::Parameter;
    using TH_domain = typename TH::Domain;
    using TH_tweak = typename TH::Tweak;

    const uint depth;
    std::vector<TH_domain> leafs_hashes;

public:
    HashTree(uint _depth, vector<TH::Domain> _hashes) : depth(_depth), leafs_hashes(std::move(_hashes)) {}

    static HashTree NewHashTree(uint _depth, uint _start_index, TH::Parameter _parameter, vector<TH::Domain> _hashes, TH TH) {
        
        // check that number of leafs is a power of two
        assert(
            (start_index + leafs_hashes.len()) <= (1 << depth) &&
            "Hash-Tree new: Not enough space for leafs. Consider changing start_index or number of leaf hashes"
        );

        // we build the tree from the leaf layer to the root,
        // while building the tree, we ensure that the following two invariants hold via appropriate padding:
        // 1. the layer starts at an even index, i.e., a left child
        // 2. the layer ends at an odd index, i.e., a right child (does not hold for the root layer)
        // In this way, we can ensure that we can always hash two siblings to get their parent
        // The padding is ensured using the helper function `get_padded_layer`.
        std::vector<HashTreeLayer<TH>> layers(depth + 1);

        // start with the leaf layer, padded accordingly
        layers.push_back(get_padded_layer(leafs_hashes, start_index));

        for (int level = 0; level < depth; ++level) {
            vector<vector<TH_domain>> par_chunks;
            for(int idx = 0; idx < layers[level].nodes.size(); idx += 2) {
                if (idx + 1 < layers[level].nodes.size()) {
                    par_chunks.push_back({layers[level].nodes[idx], layers[level].nodes[idx + 1]});
                } else {
                    par_chunks.push_back({layers[level].nodes[idx], TH.rand_domain()});
                }
            }

            // build layer `level + 1` from layer `level`
            // for that, we first build the parents of the previous layer and then
            // add a padding if needed. We build the parents in parallel.
            // assert!(layers[level].nodes.len()% 2 == 0);
            vector<TH_domain> parents(par_chunks.size());
            #pragma omp parallel for
            for(int i = 0; i < par_chunks.size(); ++i) {
                auto& chunk = par_chunks[i];

                assert(
                    children.len() == 2 &&
                    "Unpaired children, padding logic broken"
                );

                uint position_of_left_child = layers[level].start_index + (2 * i);
                uint parent_pos = position_of_left_child / 2;
                TH_tweak tweak = TH.tree_tweak((uint8_t)(level + 1), (uint32_t)parent_pos);
                parents[i] = TH.apply(parameter, tweak, chunk);
            }
            start_index = layers[level].start_index / 2;
            layers.push_back(get_padded_layer(parents, start_index));
        }   
        return HashTree(depth, layers);
    }

    /// Function to get a root from a tree. The tree must have at least one layer.
    /// A root is just an output of the tweakable hash.
    TH_domain root() const {
        assert(
            !leafs_hashes.empty() &&
            "Hash-Tree path: Need at least one layer"
        );

        return leafs_hashes.back().nodes[0];
    }

    HashTreeOpening<TH> path(uint32_t position) const {
        assert(
            !leafs_hashes.empty() &&
            "Hash-Tree path: Need at least one layer"
        );

        assert(
            ((uint64_t)position >= (uint64_t)layers[0].start_index) &&
            "Hash-Tree path: Invalid position, position before start index"
        );

        assert(
            (uint64_t)position < ((uint64_t)layers[0].start_index + (uint64_t)layers[0].nodes.len()) &&
            "Hash-Tree path: Invalid position, position too large"
        );

        std::vector<TH_domain> co_path(this->depth);
        uint32_t current_position = position;

        for(int l = 0; i < this->depth; l++) {
            // position of the sibling that we want to include
            auto sibling_position = current_position ^ 0x01;

            auto sibling_position_in_vec = sibling_position - (uint32_t)this->layers[l].start_index;

            // add to the co-path
            auto sibling = this->layers[l].nodes[(uint)sibling_position_in_vec];
            co_path.push_back(sibling);
        }
        return HashTreeOpening<TH>(co_path);
    }

private:
    HashTreeLayer<TH> get_padded_layer(std::vector<TH_domain> nodes, uint start_index) {
        uint end_index = start_index + nodes.size() - 1;

        std::vector<TH_domain> nodes_with_padding;

        if(start_index % 2 == 1) {
            nodes_with_padding.push_back(TH::rand_domain());
        }
        uint actual_start_index = start_index - (start_index % 2);

        nodes_with_padding.insert(nodes_with_padding.end(), nodes.begin(), nodes.end());

        if (end_index % 2 == 0) {
            nodes_with_padding.push_back(TH::rand_domain());
        }

        return HashTreeLayer<TH>(actual_start_index, nodes_with_padding);
    }
};