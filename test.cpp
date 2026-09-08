#include <iostream>
#include "src/inc_encoding/basic_winternitz.hpp"
#include "src/symmetric/message_hash/sha.hpp"
#include "src/signature/generalized_xmss.hpp"
#include "src/symmetric/prf/sha.hpp"
#include "src/symmetric/tweak_hash/sha.hpp"

int main() {
    ShaMessageHash<32> sha256_hasher(8, 8);
    WinternitzEncoding<ShaMessageHash<32>> winternitz(sha256_hasher, 8, 2);
    SignatureScheme<SHA256PRF, WinternitzEncoding<ShaMessageHash<32>>, ShaTweakHash, 10> sig_scheme(
        ShaTweakHash(32, 32),
        SHA256PRF(32),
        winternitz
    );

    sig_scheme.key_gen()
}
