/* poseidon2_tags.h -- domain-separation tags for the Poseidon2-BN254 tweakable hash.
 *
 * Shared by the C++ core (tweak_hash/poseidon2.hpp, message_hash/poseidon2.hpp) and the
 * Aurora verification circuit (SNARK/aurora/aggregate_aurora_verify.cpp), and mirrored
 * in Rust by SNARK/plonky3_air/src/native.rs. A tag is a u64 that goes into the sponge's
 * capacity element, so the tweak costs nothing in-circuit.
 *
 *   bits 58..63  number of absorbed field elements
 *   bits 52..54  domain (ENC / CHAIN / LEAF / NODE)
 *   bits 32..51  a  (epoch for ENC/CHAIN/LEAF, level for NODE)
 *   bits 16..31  b  (chain index for CHAIN, position-in-level for NODE)
 *   bits  0..15  c  (absolute position in chain for CHAIN)
 *
 * Fields are XORed into place, so they must fit their widths: epoch < 2^20,
 * chain index / position-in-level < 2^16, position in chain < 2^16. C/C++17 compatible.
 */
#ifndef POSEIDON2_TAGS_H
#define POSEIDON2_TAGS_H
#include <stdint.h>
#include <stddef.h>

enum { P2_DOM_ENC = 1, P2_DOM_CHAIN = 2, P2_DOM_LEAF = 3, P2_DOM_NODE = 4 };

static inline uint64_t p2_dom(uint64_t base, uint64_t a, uint64_t b, uint64_t c) {
    return (base << 52) ^ (a << 32) ^ (b << 16) ^ c;
}
static inline uint64_t p2_with_len(uint64_t tag, size_t n) { return tag ^ ((uint64_t)n << 58); }

/* message hash: H(param, rho, msg...) */
static inline uint64_t p2_tag_enc(uint32_t epoch, size_t n) {
    return p2_with_len(p2_dom(P2_DOM_ENC, epoch, 0, 0), n);
}
/* chain step producing the value at absolute position `pos`: H(param, prev) */
static inline uint64_t p2_tag_chain(uint32_t epoch, uint32_t chain_index, uint32_t pos) {
    return p2_with_len(p2_dom(P2_DOM_CHAIN, epoch, chain_index, pos), 2);
}
/* Merkle leaf (level 0) at `epoch`: H(param, chain_ends...) */
static inline uint64_t p2_tag_leaf(uint32_t epoch, size_t n) {
    return p2_with_len(p2_dom(P2_DOM_LEAF, epoch, 0, 0), n);
}
/* inner Merkle node at (level >= 1, pos): H(param, left, right) */
static inline uint64_t p2_tag_node(uint32_t level, uint32_t pos) {
    return p2_with_len(p2_dom(P2_DOM_NODE, level, pos, 0), 3);
}

#endif
