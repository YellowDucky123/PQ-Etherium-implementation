/* C ABI for the Plonky3 Poseidon2-BN254 FFI crate (src/SNARK/plonky3_ffi).
 * Field elements are 4 little-endian u64 limbs, canonical (< p).
 * All functions return 0 on success, negative on error. */
#ifndef P3_POSEIDON2_H
#define P3_POSEIDON2_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* state: 3 elements x 4 limbs = 12 u64, permuted in place. -2 if an input is >= p. */
int32_t p3_poseidon2_bn254_permute(uint64_t *state);
/* out: 64 rounds x 3 elements x 4 limbs = 768 u64 (HorizenLabs RC3 layout). */
int32_t p3_poseidon2_bn254_round_constants(uint64_t *out);
/* out: {width, sbox_degree, rounds_f, rounds_p} */
int32_t p3_poseidon2_bn254_params(uint32_t *out);

#ifdef __cplusplus
}
#endif
#endif
