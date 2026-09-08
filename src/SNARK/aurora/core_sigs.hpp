/* core_sigs.hpp -- tiny C++-standard boundary between the C++23 core scheme and
 * the C++17 libiop/Aurora code. Exposes only std::vector<uint8_t>, no core types. */
#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>

struct RealSigs {
    std::vector<std::vector<uint8_t>> pks;   // flattened public keys (root || parameter)
    std::vector<std::vector<uint8_t>> sigs;  // flattened signatures (path || rho || hashes)
};

// Generate k real Generalized-XMSS signatures of `msg` at `epoch`, verify each
// natively, and return the flattened (pk, sig) bytes. Throws if any native
// verification fails.
RealSigs make_real_sigs(std::size_t k, uint32_t epoch, const std::vector<uint8_t>& msg);
