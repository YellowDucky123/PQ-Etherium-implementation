#include <openssl/rand.h>
#include <cstdint>

template <typename Parameter_i, typename Tweak_i, typename Domain_i>
struct TweakableHash {
    using Parameter = Parameter_i;
    using Tweak = Tweak_i;
    using Domain = Domain_i;

    virtual Parameter rand_parameter() = 0;

    virtual Domain rand_domain() = 0;

    /// Returns a tweak to be used in the Merkle tree.
    /// Note: this is assumed to be distinct from the outputs of chain_tweak
    virtual Tweak tree_tweak(uint8_t level, uint32_t pos_in_level) = 0;

    /// Returns a tweak to be used in chains.
    /// Note: this is assumed to be distinct from the outputs of tree_tweak
    virtual Tweak chain_tweak(uint32_t epoch, uint8_t chain_index, uint8_t pos_in_chain) = 0;

    /// Applies the tweakable hash to parameter, tweak, and message.
    virtual Domain apply(Parameter parameter, Tweak& tweak, Domain&) = 0;

    virtual void internal_consistency_check() = 0;
};