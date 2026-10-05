/* export_p2_sigs.cpp -- produce REAL Generalized-XMSS signatures with the C++ core
 * (Poseidon2-BN254 tweak hash + message hash) and write them as an aggregation instance
 * that both provers read:
 *   - aurora/aggregate_aurora_verify   (libiop Aurora, R1CS)
 *   - plonky3_air                      (Plonky3 uni-stark, AIR)
 *
 * Every signature is verified natively before it is written.
 *
 * Usage: export_p2_sigs <k> <epoch> <out-file>
 *
 * Format (text, one record per line; field elements = 64 hex digits, big-endian):
 *   p2xmss-instance v1
 *   params <W_BITS> <DIM> <HGT> <MSG_ELEMS>
 *   k <k>
 *   epoch <epoch>
 *   msg <m_0> <m_1>
 *   signer <param> <root> <rho> <hash_0..hash_{DIM-1}> <path_0..path_{HGT-1}>     (k lines)
 *
 * Build (from repo root):
 *   g++ -std=c++23 -O2 -I. src/SNARK/export_p2_sigs.cpp src/symmetric/prf/sha.cpp \
 *       src/SNARK/plonky3_ffi/target/release/libp3_poseidon2_ffi.a -lcrypto -lpthread -ldl -lm
 */
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "src/inc_encoding/basic_winternitz.hpp"
#include "src/symmetric/message_hash/poseidon2.hpp"
#include "src/signature/generalized_xmss.hpp"
#include "src/symmetric/prf/sha.hpp"
#include "src/symmetric/tweak_hash/poseidon2.hpp"

// Must match the circuits (they check the `params` line).
static constexpr unsigned W_BITS = 2, DIM = 4, HGT = 3, MSG_ELEMS = 2;

using Enc = WinternitzEncoding<Poseidon2MessageHash>;
using Scheme = SignatureScheme<SHA256PRF, Enc, Poseidon2TweakHash, HGT>;

static std::string hex(const std::vector<uint8_t> &bytes) {
    const p2bn254::Fe x = p2bn254::from_bytes(bytes);   // canonical
    char buf[65];
    std::snprintf(buf, sizeof buf, "%016llx%016llx%016llx%016llx",
                  (unsigned long long)x[3], (unsigned long long)x[2],
                  (unsigned long long)x[1], (unsigned long long)x[0]);
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        std::fprintf(stderr, "usage: %s <k> <epoch> <out-file>\n", argv[0]);
        return 2;
    }
    const unsigned k = std::atoi(argv[1]);
    const uint32_t epoch = std::atoi(argv[2]);
    if (k == 0 || epoch >= (1u << HGT)) {
        std::fprintf(stderr, "need k >= 1 and epoch < %u\n", 1u << HGT);
        return 2;
    }

    // NUM_CHUNKS_CHECKSUM = 0: the circuits model plain Winternitz chunks (no checksum).
    Scheme scheme(Poseidon2TweakHash(), SHA256PRF(24), Enc(Poseidon2MessageHash(DIM, W_BITS), W_BITS, 0));

    std::vector<uint8_t> message(32);
    for (int i = 0; i < 32; ++i) message[i] = uint8_t(0xA0 + i);

    FILE *f = std::fopen(argv[3], "w");
    if (!f) { std::perror("fopen"); return 1; }
    std::fprintf(f, "p2xmss-instance v1\nparams %u %u %u %u\nk %u\nepoch %u\n", W_BITS, DIM, HGT, MSG_ELEMS, k, epoch);
    std::fprintf(f, "msg %s %s\n",
                 hex(std::vector<uint8_t>(message.begin(), message.begin() + 16)).c_str(),
                 hex(std::vector<uint8_t>(message.begin() + 16, message.end())).c_str());

    for (unsigned s = 0; s < k; ++s) {
        auto [pk, sk] = scheme.key_gen(0, 1u << HGT);
        auto sig = scheme.sign(sk, epoch, message);
        if (!scheme.verify(pk, epoch, message, sig)) {
            std::fprintf(stderr, "signer %u: native verification FAILED\n", s);
            return 1;
        }
        std::fprintf(f, "signer %s %s %s", hex(pk.parameter).c_str(), hex(pk.root).c_str(), hex(sig.rho).c_str());
        for (const auto &h : sig.hashes) std::fprintf(f, " %s", hex(h).c_str());
        for (const auto &p : sig.path.co_path) std::fprintf(f, " %s", hex(p).c_str());
        std::fprintf(f, "\n");
    }
    std::fclose(f);
    std::printf("[export] %u real Poseidon2 signatures (epoch %u) verified natively -> %s\n", k, epoch, argv[3]);
    return 0;
}
