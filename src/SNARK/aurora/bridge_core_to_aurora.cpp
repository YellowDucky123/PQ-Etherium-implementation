/*  bridge_core_to_aurora.cpp  (compiled as C++17, linked with core_sigs.o)
 *
 *  End-to-end link between the REAL Generalized-XMSS core and the Aurora
 *  aggregation: real key_gen/sign/verify happens in core_sigs.cpp (C++23); here
 *  we take the flattened (pk, sig) bytes and feed them to the Aurora aggregation
 *  SNARK -- proving the fixed core actually plugs into the SNARK layer.
 */
#include <cstdint>
#include <vector>
#include <cstdio>

#include "core_sigs.hpp"

#define AGG_AURORA_NO_MAIN
#include "aggregate_aurora.cpp"

int main() {
    const std::size_t K = 4;
    const uint32_t    epoch = 3;
    std::vector<uint8_t> message(32, 0x42);

    RealSigs rs = make_real_sigs(K, epoch, message);
    printf("[bridge] %zu real signatures generated + natively verified "
           "(pk=%zu B, sig=%zu B each)\n", K, rs.pks[0].size(), rs.sigs[0].size());

    libiop::stmnt_T statement{K, epoch, message, rs.pks};
    libiop::witn_T  witness = rs.sigs;
    const bool ok = libiop::aggregate(statement, witness);

    printf("[bridge] Aurora aggregation over REAL signatures: %s\n", ok ? "SUCCESS" : "FAILED");
    return ok ? 0 : 1;
}
