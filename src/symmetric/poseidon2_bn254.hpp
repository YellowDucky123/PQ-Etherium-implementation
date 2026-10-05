#pragma once
/* Poseidon2-BN254 sponge for the C++ core.
 *
 * The permutation is Plonky3's Poseidon2Bn254<3> (t=3, x^5, R_F=8, R_P=56, HorizenLabs
 * constants) called over the C ABI in ../SNARK/plonky3_ffi -- link
 * libp3_poseidon2_ffi.a. This is the same permutation the Aurora R1CS gadget and the
 * Plonky3 AIR are checked against, so signatures made here verify in both circuits.
 *
 * Field elements live in the core as 32-byte little-endian byte strings (the core's
 * Domain/Parameter type is std::vector<uint8_t>). Shorter strings (e.g. 24-byte PRF
 * outputs) are read as LE integers; anything >= p is reduced mod p.
 *
 * Sponge: rate 2, capacity 1. state = [0, 0, tag]; absorb inputs two at a time into
 * state[0], state[1], permute after each pair (a trailing odd input is absorbed alone);
 * output state[0].
 */
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <openssl/rand.h>

#include "../SNARK/plonky3_ffi/p3_poseidon2.h"

namespace p2bn254 {

using Fe = std::array<uint64_t, 4>;   // canonical, little-endian limbs

// BN254 scalar field modulus r
inline constexpr Fe P = {0x43e1f593f0000001ULL, 0x2833e84879b97091ULL,
                         0xb85045b68181585dULL, 0x30644e72e131a029ULL};

inline bool geq(const Fe &a, const Fe &b) {
    for (int i = 3; i >= 0; --i) {
        if (a[i] != b[i]) return a[i] > b[i];
    }
    return true;
}

inline void sub_in_place(Fe &a, const Fe &b) {
    unsigned __int128 borrow = 0;
    for (int i = 0; i < 4; ++i) {
        unsigned __int128 d = (unsigned __int128)a[i] - b[i] - borrow;
        a[i] = (uint64_t)d;
        borrow = (d >> 64) ? 1 : 0;
    }
}

inline Fe add(const Fe &a, const Fe &b) {
    Fe r;
    unsigned __int128 carry = 0;
    for (int i = 0; i < 4; ++i) {
        unsigned __int128 s = (unsigned __int128)a[i] + b[i] + carry;
        r[i] = (uint64_t)s;
        carry = s >> 64;
    }
    // a, b < p < 2^254, so the sum fits in 255 bits (no carry out) and one subtraction suffices
    if (geq(r, P)) sub_in_place(r, P);
    return r;
}

inline Fe from_u64(uint64_t v) { return Fe{v, 0, 0, 0}; }

inline Fe from_bytes(const std::vector<uint8_t> &bytes) {
    if (bytes.size() > 32) throw std::invalid_argument("p2bn254: field element longer than 32 bytes");
    uint8_t buf[32] = {0};
    std::memcpy(buf, bytes.data(), bytes.size());
    Fe r;
    for (int i = 0; i < 4; ++i) {
        uint64_t limb = 0;
        for (int j = 7; j >= 0; --j) limb = (limb << 8) | buf[8 * i + j];
        r[i] = limb;
    }
    while (geq(r, P)) sub_in_place(r, P);   // 2^256 / p < 6
    return r;
}

inline std::vector<uint8_t> to_bytes(const Fe &x) {
    std::vector<uint8_t> out(32);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 8; ++j) out[8 * i + j] = (uint8_t)(x[i] >> (8 * j));
    return out;
}

// Uniform-ish canonical element: 253 random bits (< 2^253 < p).
inline std::vector<uint8_t> rand_bytes() {
    std::vector<uint8_t> out(32);
    if (RAND_bytes(out.data(), 32) != 1) throw std::runtime_error("p2bn254: RAND_bytes failed");
    out[31] &= 0x1f;
    return out;
}

inline void permute(std::array<Fe, 3> &s) {
    uint64_t buf[12];
    for (int i = 0; i < 3; ++i) std::memcpy(buf + 4 * i, s[i].data(), 32);
    if (p3_poseidon2_bn254_permute(buf) != 0) throw std::runtime_error("p2bn254: permute failed");
    for (int i = 0; i < 3; ++i) std::memcpy(s[i].data(), buf + 4 * i, 32);
}

inline Fe sponge(const std::vector<Fe> &ins, uint64_t tag) {
    std::array<Fe, 3> s{Fe{}, Fe{}, from_u64(tag)};
    for (size_t k = 0; k < ins.size(); k += 2) {
        s[0] = add(s[0], ins[k]);
        if (k + 1 < ins.size()) s[1] = add(s[1], ins[k + 1]);
        permute(s);
    }
    return s[0];
}

}  // namespace p2bn254
