#include <iostream>
#include <vector>
#include "src/inc_encoding/basic_winternitz.hpp"
#include "src/symmetric/message_hash/sha.hpp"
#include "src/signature/generalized_xmss.hpp"
#include "src/symmetric/prf/sha.hpp"
#include "src/symmetric/tweak_hash/sha.hpp"

using Scheme = SignatureScheme<SHA256PRF, WinternitzEncoding<ShaMessageHash<32>>, ShaTweakHash, 4>;

int main() {
    Scheme scheme(ShaTweakHash(24, 24), SHA256PRF(24),
                  WinternitzEncoding<ShaMessageHash<32>>(ShaMessageHash<32>(8, 8), 8, 2));
    auto [pk, sk] = scheme.key_gen(0, 8);   // lifetime 2^4=16, activate epochs 0..7

    int pass = 0, fail = 0;
    auto check = [&](bool cond, const char* name){ (cond?pass:fail)++; std::cout << (cond?"[PASS] ":"[FAIL] ") << name << "\n"; };

    // 1) valid signatures verify at several epochs
    for (uint32_t ep : {0u, 3u, 7u}) {
        std::vector<uint8_t> msg(32); for (int i=0;i<32;i++) msg[i]=uint8_t(i+ep);
        auto sig = scheme.sign(sk, ep, msg);
        check(scheme.verify(pk, ep, msg, sig), "valid signature verifies");

        // 2) wrong message rejected
        auto bad_msg = msg; bad_msg[0] ^= 0xFF;
        check(!scheme.verify(pk, ep, bad_msg, sig), "wrong message rejected");

        // 3) tampered signature (flip a hash byte) rejected
        auto h = sig.hashes; if (!h.empty() && !h[0].empty()) h[0][0] ^= 0xFF;
        Scheme::Signature bad_sig(sig.path, sig.rho, h);
        check(!scheme.verify(pk, ep, msg, bad_sig), "tampered signature rejected");

        // 4) wrong epoch rejected
        check(!scheme.verify(pk, (ep+1)%8, msg, sig), "wrong epoch rejected");
    }
    std::cout << "\n" << pass << " passed, " << fail << " failed\n";
    return fail == 0 ? 0 : 1;
}
