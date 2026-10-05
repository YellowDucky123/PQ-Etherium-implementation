//! STARK configuration over the BN254 scalar field.
//!
//! - Commitments: Merkle trees with BLAKE3 (field elements hashed as 32-byte canonical LE),
//!   the counterpart of Aurora's BLAKE2b BCS Merkle trees.
//! - Fiat-Shamir: a small BLAKE3 transcript. Plonky3's DuplexChallenger only implements
//!   bit sampling for 64-bit fields, so BN254 needs its own challenger.
//! - Challenges are drawn from BN254 itself (|F| ~ 2^254, no extension field needed).

use num_bigint::BigUint;
use p3_bn254::Bn254;
use p3_challenger::{CanObserve, CanSample, CanSampleBits, FieldChallenger, GrindingChallenger};
use p3_commit::ExtensionMmcs;
use p3_dft::Radix2DitParallel;
use p3_field::{Field, PrimeCharacteristicRing, PrimeField};
use p3_fri::{FriParameters, TwoAdicFriPcs};
use p3_merkle_tree::MerkleTreeMmcs;
use p3_symmetric::{CompressionFunctionFromHasher, CryptographicHasher, Hash};
use p3_uni_stark::StarkConfig;

pub type Val = Bn254;
pub type Challenge = Bn254;

fn canonical_bytes(x: &Bn254) -> [u8; 32] {
    let mut out = [0u8; 32];
    let b = x.as_canonical_biguint().to_bytes_le();
    out[..b.len()].copy_from_slice(&b);
    out
}

/// BLAKE3 over a stream of BN254 elements (each as 32 canonical LE bytes).
#[derive(Clone, Copy, Debug, Default)]
pub struct FieldBlake3;

impl CryptographicHasher<Bn254, [u8; 32]> for FieldBlake3 {
    fn hash_iter<I: IntoIterator<Item = Bn254>>(&self, input: I) -> [u8; 32] {
        let mut h = blake3::Hasher::new();
        for x in input {
            h.update(&canonical_bytes(&x));
        }
        *h.finalize().as_bytes()
    }
}

pub type ValMmcs = MerkleTreeMmcs<Val, u8, FieldBlake3, CompressionFunctionFromHasher<p3_blake3::Blake3, 2, 32>, 32>;
pub type ChallengeMmcs = ExtensionMmcs<Val, Challenge, ValMmcs>;
pub type Pcs = TwoAdicFriPcs<Val, Radix2DitParallel<Val>, ValMmcs, ChallengeMmcs>;
pub type Config = StarkConfig<Pcs, Challenge, Blake3Challenger>;

/// Hash-chain Fiat-Shamir transcript.
/// observe: bytes are appended to a pending buffer.
/// sample:  state <- H(state || pending) if anything is pending, then output
///          H(state || counter) (counter reset whenever new data is absorbed).
#[derive(Clone, Debug)]
pub struct Blake3Challenger {
    state: [u8; 32],
    pending: Vec<u8>,
    ctr: u64,
}

impl Blake3Challenger {
    pub fn new(domain: &[u8]) -> Self {
        Self { state: *blake3::hash(domain).as_bytes(), pending: Vec::new(), ctr: 0 }
    }

    fn squeeze(&mut self) -> [u8; 32] {
        if !self.pending.is_empty() {
            let mut h = blake3::Hasher::new();
            h.update(&self.state);
            h.update(&self.pending);
            self.state = *h.finalize().as_bytes();
            self.pending.clear();
            self.ctr = 0;
        }
        let mut h = blake3::Hasher::new();
        h.update(&self.state);
        h.update(&self.ctr.to_le_bytes());
        self.ctr += 1;
        *h.finalize().as_bytes()
    }
}

impl CanObserve<Bn254> for Blake3Challenger {
    fn observe(&mut self, value: Bn254) {
        self.pending.extend_from_slice(&canonical_bytes(&value));
    }
}

impl CanObserve<Hash<Bn254, u8, 32>> for Blake3Challenger {
    fn observe(&mut self, value: Hash<Bn254, u8, 32>) {
        let bytes: [u8; 32] = value.into();
        self.pending.extend_from_slice(&bytes);
    }
}

impl CanSample<Bn254> for Blake3Challenger {
    fn sample(&mut self) -> Bn254 {
        // 512 bits reduced mod p: statistical distance ~2^-258 from uniform
        let mut wide = [0u8; 64];
        wide[..32].copy_from_slice(&self.squeeze());
        wide[32..].copy_from_slice(&self.squeeze());
        let v = BigUint::from_bytes_le(&wide) % Bn254::order();
        Bn254::from_biguint(v).unwrap()
    }
}

impl CanSampleBits<usize> for Blake3Challenger {
    fn sample_bits(&mut self, bits: usize) -> usize {
        assert!(bits < usize::BITS as usize);
        let b = self.squeeze();
        let v = usize::from_le_bytes(b[..8].try_into().unwrap());
        v & ((1usize << bits) - 1)
    }
}

impl FieldChallenger<Bn254> for Blake3Challenger {}

impl GrindingChallenger for Blake3Challenger {
    type Witness = Bn254;

    fn grind(&mut self, bits: usize) -> Bn254 {
        let witness = (0u64..)
            .map(Bn254::from_u64)
            .find(|w| self.clone().check_witness(bits, *w))
            .expect("grinding failed");
        assert!(self.check_witness(bits, witness));
        witness
    }
}

pub struct Params {
    pub log_blowup: usize,
    pub num_queries: usize,
    pub pow_bits: usize,
}

pub fn make_config(p: &Params) -> Config {
    let mmcs = ValMmcs::new(FieldBlake3, CompressionFunctionFromHasher::new(p3_blake3::Blake3));
    let fri = FriParameters {
        log_blowup: p.log_blowup,
        log_final_poly_len: 0,
        num_queries: p.num_queries,
        commit_proof_of_work_bits: 0,
        query_proof_of_work_bits: p.pow_bits,
        mmcs: ChallengeMmcs::new(mmcs.clone()),
    };
    let pcs = Pcs::new(Radix2DitParallel::default(), mmcs, fri);
    Config::new(pcs, Blake3Challenger::new(b"p3-xmss-agg/bn254/v1"))
}
