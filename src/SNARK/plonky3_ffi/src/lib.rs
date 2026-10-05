//! C ABI over Plonky3's Poseidon2 permutation for the BN254 scalar field.
//!
//! Instance: width t=3, S-box x^5, R_F=8 full rounds, R_P=56 partial rounds, with the
//! HorizenLabs round constants (`constants.rs`). This is the same instance Plonky3 tests
//! against the HorizenLabs reference (`p3-bn254::poseidon2::tests::test_poseidon2_bn254`).
//!
//! Field elements cross the boundary as 4 little-endian u64 limbs in CANONICAL form
//! (i.e. the integer in [0, p)), which is what libff's `bigint<4>` uses as well.

mod constants;

use std::sync::OnceLock;

use num_bigint::BigUint;
use p3_bn254::{Bn254, Poseidon2Bn254};
use p3_field::{Field, PrimeField};
use p3_poseidon2::ExternalLayerConstants;
use p3_symmetric::Permutation;

pub const WIDTH: usize = 3;
pub const SBOX_DEGREE: u32 = 5;
pub const ROUNDS_F: usize = 8;
pub const ROUNDS_P: usize = 56;
pub const LIMBS: usize = 4;

fn from_hex(h: &str) -> Bn254 {
    Bn254::from_biguint(BigUint::parse_bytes(h.as_bytes(), 16).unwrap()).unwrap()
}

/// All 64 rows of round constants as field elements (row layout as in `constants.rs`).
pub fn round_constants() -> Vec<[Bn254; WIDTH]> {
    constants::RC3_HEX.iter().map(|row| row.map(from_hex)).collect()
}

pub fn perm() -> &'static Poseidon2Bn254<WIDTH> {
    static PERM: OnceLock<Poseidon2Bn254<WIDTH>> = OnceLock::new();
    PERM.get_or_init(|| {
        let mut rc = round_constants();
        let internal: Vec<Bn254> =
            rc.drain(ROUNDS_F / 2..ROUNDS_F / 2 + ROUNDS_P).map(|r| r[0]).collect();
        let external =
            ExternalLayerConstants::new(rc[..ROUNDS_F / 2].to_vec(), rc[ROUNDS_F / 2..].to_vec());
        Poseidon2Bn254::<WIDTH>::new(external, internal)
    })
}

fn modulus() -> &'static BigUint {
    static P: OnceLock<BigUint> = OnceLock::new();
    P.get_or_init(Bn254::order)
}

/// Canonical limbs -> field element; None if the integer is >= p.
/// (`Bn254::from_biguint` silently reduces mod p, so the range check is done here.)
pub fn from_limbs(l: &[u64]) -> Option<Bn254> {
    let mut bytes = [0u8; 32];
    for (i, limb) in l.iter().enumerate() {
        bytes[8 * i..8 * i + 8].copy_from_slice(&limb.to_le_bytes());
    }
    let v = BigUint::from_bytes_le(&bytes);
    if &v >= modulus() {
        return None;
    }
    Bn254::from_biguint(v)
}

pub fn to_limbs(x: Bn254, out: &mut [u64]) {
    let digits = x.as_canonical_biguint().to_u64_digits();
    for (i, o) in out.iter_mut().enumerate() {
        *o = digits.get(i).copied().unwrap_or(0);
    }
}

/// Permute `state` (WIDTH*LIMBS u64s) in place.
/// Returns 0 on success, -1 on a null pointer, -2 if an input limb-set is not < p
/// (state is left untouched on error).
///
/// # Safety
/// `state` must point to WIDTH*LIMBS = 12 writable u64s.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn p3_poseidon2_bn254_permute(state: *mut u64) -> i32 {
    if state.is_null() {
        return -1;
    }
    let s = unsafe { std::slice::from_raw_parts_mut(state, WIDTH * LIMBS) };
    let mut st = [Bn254::default(); WIDTH];
    for i in 0..WIDTH {
        match from_limbs(&s[i * LIMBS..(i + 1) * LIMBS]) {
            Some(x) => st[i] = x,
            None => return -2,
        }
    }
    perm().permute_mut(&mut st);
    for i in 0..WIDTH {
        to_limbs(st[i], &mut s[i * LIMBS..(i + 1) * LIMBS]);
    }
    0
}

/// Write the 64x3 round-constant table (64*WIDTH*LIMBS u64s, row-major) to `out`,
/// so a circuit can use exactly the constants the native permutation uses.
///
/// # Safety
/// `out` must point to 64*WIDTH*LIMBS = 768 writable u64s.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn p3_poseidon2_bn254_round_constants(out: *mut u64) -> i32 {
    if out.is_null() {
        return -1;
    }
    let o = unsafe { std::slice::from_raw_parts_mut(out, 64 * WIDTH * LIMBS) };
    for (r, row) in round_constants().into_iter().enumerate() {
        for (c, x) in row.into_iter().enumerate() {
            let at = (r * WIDTH + c) * LIMBS;
            to_limbs(x, &mut o[at..at + LIMBS]);
        }
    }
    0
}

/// Write {WIDTH, SBOX_DEGREE, ROUNDS_F, ROUNDS_P} to `out`.
///
/// # Safety
/// `out` must point to 4 writable u32s.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn p3_poseidon2_bn254_params(out: *mut u32) -> i32 {
    if out.is_null() {
        return -1;
    }
    let o = unsafe { std::slice::from_raw_parts_mut(out, 4) };
    o.copy_from_slice(&[WIDTH as u32, SBOX_DEGREE, ROUNDS_F as u32, ROUNDS_P as u32]);
    0
}

#[cfg(test)]
mod tests {
    use super::*;
    use ark_ff::{BigInteger, PrimeField as ArkPrimeField};
    use rand::{Rng, SeedableRng, rngs::StdRng};
    use zkhash::fields::bn256::FpBN256;
    use zkhash::poseidon2::poseidon2::Poseidon2 as Poseidon2Ref;
    use zkhash::poseidon2::poseidon2_instance_bn256::{POSEIDON2_BN256_PARAMS, RC3};

    fn ark_to_p3(x: FpBN256) -> Bn254 {
        Bn254::from_biguint(BigUint::from_bytes_le(&x.into_bigint().to_bytes_le())).unwrap()
    }
    fn p3_to_ark(x: Bn254) -> FpBN256 {
        FpBN256::from_le_bytes_mod_order(&x.as_canonical_biguint().to_bytes_le())
    }

    #[test]
    fn constants_match_zkhash() {
        let ours = round_constants();
        assert_eq!(ours.len(), RC3.len());
        for (a, b) in ours.iter().zip(RC3.iter()) {
            for c in 0..WIDTH {
                assert_eq!(a[c], ark_to_p3(b[c]));
            }
        }
    }

    #[test]
    fn permutation_matches_zkhash() {
        let reference = Poseidon2Ref::new(&POSEIDON2_BN256_PARAMS);
        let mut rng = StdRng::seed_from_u64(7);
        for _ in 0..32 {
            let mut limbs = [0u64; WIDTH * LIMBS];
            let mut input = [Bn254::default(); WIDTH];
            for i in 0..WIDTH {
                let mut l = [0u64; LIMBS];
                rng.fill(&mut l[..]);
                l[3] &= 0x0fff_ffff_ffff_ffff; // < 2^252 < p
                input[i] = from_limbs(&l).unwrap();
                limbs[i * LIMBS..(i + 1) * LIMBS].copy_from_slice(&l);
            }
            let expected: Vec<Bn254> = reference
                .permutation(&input.map(p3_to_ark))
                .into_iter()
                .map(ark_to_p3)
                .collect();
            assert_eq!(unsafe { p3_poseidon2_bn254_permute(limbs.as_mut_ptr()) }, 0);
            for i in 0..WIDTH {
                assert_eq!(from_limbs(&limbs[i * LIMBS..(i + 1) * LIMBS]).unwrap(), expected[i]);
            }
        }
    }

    #[test]
    fn rejects_non_canonical() {
        let mut limbs = [u64::MAX; WIDTH * LIMBS];
        assert_eq!(unsafe { p3_poseidon2_bn254_permute(limbs.as_mut_ptr()) }, -2);
    }
}
