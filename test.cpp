// Minimal Generalized-XMSS sign/verify round-trip.
// Build:  g++ -std=c++23 -O2 -I. test.cpp src/symmetric/prf/sha.cpp -lcrypto -o test && ./test
#include <iostream>
#include <vector>
#include "src/inc_encoding/basic_winternitz.hpp"
#include "src/symmetric/message_hash/sha.hpp"
#include "src/signature/generalized_xmss.hpp"
#include "src/symmetric/prf/sha.hpp"
#include "src/symmetric/tweak_hash/sha.hpp"

int main() {
    SignatureScheme<SHA256PRF, WinternitzEncoding<ShaMessageHash<32>>, ShaTweakHash, 4> scheme(
        ShaTweakHash(24, 24), SHA256PRF(24),
        WinternitzEncoding<ShaMessageHash<32>>(ShaMessageHash<32>(8, 8), 8, 2));

    auto [pk, sk] = scheme.key_gen(0, 8);          // lifetime 2^4, activate epochs 0..7
    std::vector<uint8_t> message(32, 0x42);
    auto sig = scheme.sign(sk, 3, message);
    bool ok = scheme.verify(pk, 3, message, sig);
    std::cout << "verify: " << (ok ? "OK" : "FAIL") << "\n";
    return ok ? 0 : 1;
}
