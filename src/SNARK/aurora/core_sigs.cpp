/* core_sigs.cpp -- compiled as C++23 (concepts + std::byteswap). */
#include "core_sigs.hpp"
#include <stdexcept>

#include "../../inc_encoding/basic_winternitz.hpp"
#include "../../symmetric/message_hash/sha.hpp"
#include "../../signature/generalized_xmss.hpp"
#include "../../symmetric/prf/sha.hpp"
#include "../../symmetric/tweak_hash/sha.hpp"

using Scheme = SignatureScheme<SHA256PRF, WinternitzEncoding<ShaMessageHash<32>>, ShaTweakHash, 4>;

RealSigs make_real_sigs(std::size_t k, uint32_t epoch, const std::vector<uint8_t>& msg) {
    Scheme scheme(ShaTweakHash(24, 24), SHA256PRF(24),
                  WinternitzEncoding<ShaMessageHash<32>>(ShaMessageHash<32>(8, 8), 8, 2));

    RealSigs out;
    std::vector<uint8_t> message = msg;
    for (std::size_t s = 0; s < k; ++s) {
        auto [pk, sk] = scheme.key_gen(0, 8);          // fresh signer, epochs 0..7 active
        auto sig = scheme.sign(sk, epoch, message);
        if (!scheme.verify(pk, epoch, message, sig))
            throw std::runtime_error("core_sigs: native verification failed");

        std::vector<uint8_t> pkf = pk.root;
        pkf.insert(pkf.end(), pk.parameter.begin(), pk.parameter.end());
        out.pks.push_back(std::move(pkf));
        out.sigs.push_back(sig.flatten());
    }
    return out;
}
