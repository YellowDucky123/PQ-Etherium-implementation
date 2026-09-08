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

    HashTreeLayer() : start_index(0), nodes() {}
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

    std::vector<uint8_t> flatten() const {
        std::vector<uint8_t> out;
        for(const auto &cp : co_path) {
            out.insert(out.end(), cp.begin(), cp.end());
        }
        return out;
    }
};

template <TweakableHash_c TH>
class HashTree {
    using TH_parameter = typename TH::Parameter;
    using TH_domain = typename TH::Domain;
    using TH_tweak = typename TH::Tweak;

    const uint depth;
    std::vector<HashTreeLayer<TH>> layers;

    static HashTreeLayer<TH> get_padded_layer(std::vector<TH_domain> nodes, uint start_index, TH &th) {
        uint end_index = start_index + nodes.size() - 1;

        std::vector<TH_domain> nodes_with_padding;

        if(start_index % 2 == 1) {
            nodes_with_padding.push_back(th.rand_domain());
        }
        uint actual_start_index = start_index - (start_index % 2);

        nodes_with_padding.insert(nodes_with_padding.end(), nodes.begin(), nodes.end());

        if (end_index % 2 == 0) {
            nodes_with_padding.push_back(th.rand_domain());
        }

        return HashTreeLayer<TH>(actual_start_index, nodes_with_padding);
    }

public:
    HashTree(uint _depth, std::vector<HashTreeLayer<TH>> _layers) : depth(_depth), layers(std::move(_layers)) {}

    static HashTree NewHashTree(uint depth, uint start_index, TH::Parameter _parameter, std::vector<typename TH::Domain> leafs_hashes, TH th) {

        // check that number of leafs is a power of two
        assert(
            (start_index + leafs_hashes.size()) <= (1u << depth) &&
            "Hash-Tree new: Not enough space for leafs. Consider changing start_index or number of leaf hashes"
        );

        // we build the tree from the leaf layer to the root,
        // while building the tree, we ensure that the following two invariants hold via appropriate padding:
        // 1. the layer starts at an even index, i.e., a left child
        // 2. the layer ends at an odd index, i.e., a right child (does not hold for the root layer)
        // In this way, we can ensure that we can always hash two siblings to get their parent
        // The padding is ensured using the helper function `get_padded_layer`.
        std::vector<HashTreeLayer<TH>> layers;
        layers.reserve(depth + 1);

        // start with the leaf layer, padded accordingly
        layers.push_back(get_padded_layer(leafs_hashes, start_index, th));

        for (uint level = 0; level < depth; ++level) {
            std::vector<std::vector<TH_domain>> par_chunks;
            for(std::size_t idx = 0; idx < layers[level].nodes.size(); idx += 2) {
                if (idx + 1 < layers[level].nodes.size()) {
                    par_chunks.push_back({layers[level].nodes[idx], layers[level].nodes[idx + 1]});
                } else {
                    par_chunks.push_back({layers[level].nodes[idx], th.rand_domain()});
                }
            }

            // build layer `level + 1` from layer `level`
            std::vector<TH_domain> parents(par_chunks.size());
            for(std::size_t i = 0; i < par_chunks.size(); ++i) {
                std::vector<TH_domain> &children = par_chunks[i];

                assert(
                    children.size() == 2 &&
                    "Unpaired children, padding logic broken"
                );

                uint position_of_left_child = layers[level].start_index + (2 * i);
                uint parent_pos = position_of_left_child / 2;
                auto tweak = th.tree_tweak((uint8_t)(level + 1), (uint32_t)parent_pos);
                parents[i] = th.apply(_parameter, *tweak, children);
            }
            uint parent_start_index = layers[level].start_index / 2;
            layers.push_back(get_padded_layer(parents, parent_start_index, th));
        }
        return HashTree(depth, layers);
    }

    /// Function to get a root from a tree. The tree must have at least one layer.
    /// A root is just an output of the tweakable hash.
    TH_domain root() const {
        assert(
            !layers.empty() &&
            "Hash-Tree path: Need at least one layer"
        );

        return layers.back().nodes[0];
    }

    HashTreeOpening<TH> path(uint32_t position) const {
        assert(
            !layers.empty() &&
            "Hash-Tree path: Need at least one layer"
        );

        assert(
            ((uint64_t)position >= (uint64_t)layers[0].start_index) &&
            "Hash-Tree path: Invalid position, position before start index"
        );

        assert(
            (uint64_t)position < ((uint64_t)layers[0].start_index + (uint64_t)layers[0].nodes.size()) &&
            "Hash-Tree path: Invalid position, position too large"
        );

        std::vector<TH_domain> co_path;
        co_path.reserve(this->depth);
        uint32_t current_position = position;

        for(uint l = 0; l < this->depth; l++) {
            // position of the sibling that we want to include
            uint32_t sibling_position = current_position ^ 0x01;

            uint32_t sibling_position_in_vec = sibling_position - (uint32_t)this->layers[l].start_index;

            // add to the co-path
            co_path.push_back(this->layers[l].nodes[(uint)sibling_position_in_vec]);

            // move up one level
            current_position >>= 1;
        }
        return HashTreeOpening<TH>(co_path);
    }
};

template <TweakableHash_c TH>
bool hash_tree_verify(
    const typename TH::Parameter &parameter,
    const typename TH::Domain &root,
    uint32_t position,
    std::vector<typename TH::Domain> &leaf,
    const HashTreeOpening<TH> &opening,
    TH th
) {
    using TH_domain = typename TH::Domain;

    int depth = opening.co_path.size();

    assert(
        depth <= 32 &&
        "Hash-Tree verify: Tree depth must be at most 32"
    );

    // leaf hash: H(param, tree_tweak(0, position), chain_ends)
    auto leaf_tweak = th.tree_tweak(0, position);
    TH_domain current_node = th.apply(parameter, *leaf_tweak, leaf);

    uint32_t current_position = position;

    for(int l = 0; l < depth; l++) {
        std::vector<TH_domain> children(2);

        if(current_position % 2 == 0) {
            children[0] = current_node;
            children[1] = opening.co_path[l];
        } else {
            children[0] = opening.co_path[l];
            children[1] = current_node;
        }

        uint32_t parent_position = current_position >> 1;

        auto tweak = th.tree_tweak(static_cast<uint8_t>(l + 1), parent_position);
        current_node = th.apply(parameter, *tweak, children);

        current_position = parent_position;
    }

    return current_node == root;
}
