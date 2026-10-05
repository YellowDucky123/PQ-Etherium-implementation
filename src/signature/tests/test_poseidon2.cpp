// Generalized-XMSS round-trip with the Poseidon2-BN254 tweak hash + message hash.
// Build (from repo root, after `cargo build --release` in src/SNARK/plonky3_ffi):
//   g++ -std=c++23 -O2 -I. src/signature/tests/test_poseidon2.cpp src/symmetric/prf/sha.cpp \
//       src/SNARK/plonky3_ffi/target/release/libp3_poseidon2_ffi.a -lcrypto -lpthread -ldl -lm \
//       -o test_poseidon2 && ./test_poseidon2
#include <iostream>
#include <vector>
#include "src/inc_encoding/basic_winternitz.hpp"
#include "src/symmetric/message_hash/poseidon2.hpp"
#include "src/signature/generalized_xmss.hpp"
#include "src/symmetric/prf/sha.hpp"
#include "src/symmetric/tweak_hash/poseidon2.hpp"

using Enc = WinternitzEncoding<Poseidon2MessageHash>;

static int pass = 0, fail = 0;
static void check(bool cond, const char *name) {
    (cond ? pass : fail)++;
    std::cout << (cond ? "[PASS] " : "[FAIL] ") << name << "\n";
}

template <typename Scheme>
static void run(Scheme &scheme, uint32_t num_epochs, bool test_wrong_message) {
    auto [pk, sk] = scheme.key_gen(0, num_epochs);
    for (uint32_t ep : {0u, 3u, num_epochs - 1}) {
        std::vector<uint8_t> msg(32);
        for (int i = 0; i < 32; i++) msg[i] = uint8_t(i + ep);
        auto sig = scheme.sign(sk, ep, msg);
        check(scheme.verify(pk, ep, msg, sig), "valid signature verifies");

        if (test_wrong_message) {
            auto bad_msg = msg;
            bad_msg[0] ^= 0xFF;
            check(!scheme.verify(pk, ep, bad_msg, sig), "wrong message rejected");
        }

        auto h = sig.hashes;
        h[0][0] ^= 0x01;
        typename Scheme::Signature bad_sig(sig.path, sig.rho, h);
        check(!scheme.verify(pk, ep, msg, bad_sig), "tampered chain value rejected");

        auto cp = sig.path.co_path;
        cp.back()[0] ^= 0x01;
        typename Scheme::Signature bad_path(HashTreeOpening<Poseidon2TweakHash>(cp), sig.rho, sig.hashes);
        check(!scheme.verify(pk, ep, msg, bad_path), "tampered Merkle path rejected");

        check(!scheme.verify(pk, (ep + 1) % num_epochs, msg, sig), "wrong epoch rejected");
    }
}

int main() {
    std::cout << "== 128-bit message digest, w=2, 64+4 chains (with checksum), lifetime 2^4 ==\n";
    {
        SignatureScheme<SHA256PRF, Enc, Poseidon2TweakHash, 4> scheme(
            Poseidon2TweakHash(), SHA256PRF(24), Enc(Poseidon2MessageHash(64, 2), 2, 4));
        run(scheme, 8, true);
    }

    // The parameters the Aurora/Plonky3 aggregation circuits use. No checksum and only an
    // 8-bit digest, so a wrong message collides with probability 2^-8: not tested here.
    std::cout << "\n== circuit demo params: w=2, 4 chains, no checksum, lifetime 2^3 ==\n";
    {
        SignatureScheme<SHA256PRF, Enc, Poseidon2TweakHash, 3> scheme(
            Poseidon2TweakHash(), SHA256PRF(24), Enc(Poseidon2MessageHash(4, 2), 2, 0));
        run(scheme, 8, false);
    }

    std::cout << "\n" << pass << " passed, " << fail << " failed\n";
    return fail == 0 ? 0 : 1;
}
