//! Native (out-of-circuit) side: the instance file written by `export_p2_sigs`, the tweak
//! tags (a mirror of src/symmetric/tweak_hash/poseidon2_tags.h), and the Poseidon2 sponge
//! using Plonky3's own `Poseidon2Bn254<3>` from the plonky3_ffi crate.

use num_bigint::BigUint;
use p3_bn254::Bn254;
use p3_field::{Field, PrimeCharacteristicRing, PrimeField};
use p3_symmetric::Permutation;

pub const W_BITS: usize = 2;
pub const BASE: usize = 1 << W_BITS;
pub const MAXSTEPS: usize = BASE - 1;
pub const DIM: usize = 4;
pub const HGT: usize = 3;
pub const MSG_ELEMS: usize = 2;

// ---- tags: keep in sync with poseidon2_tags.h ----
const DOM_ENC: u64 = 1;
const DOM_CHAIN: u64 = 2;
const DOM_LEAF: u64 = 3;
const DOM_NODE: u64 = 4;
fn dom(base: u64, a: u64, b: u64, c: u64) -> u64 {
    (base << 52) ^ (a << 32) ^ (b << 16) ^ c
}
fn with_len(tag: u64, n: usize) -> u64 {
    tag ^ ((n as u64) << 58)
}
pub fn tag_enc(epoch: u64, n: usize) -> u64 {
    with_len(dom(DOM_ENC, epoch, 0, 0), n)
}
pub fn tag_chain(epoch: u64, chain: u64, pos: u64) -> u64 {
    with_len(dom(DOM_CHAIN, epoch, chain, pos), 2)
}
pub fn tag_leaf(epoch: u64, n: usize) -> u64 {
    with_len(dom(DOM_LEAF, epoch, 0, 0), n)
}
pub fn tag_node(level: u64, pos: u64) -> u64 {
    with_len(dom(DOM_NODE, level, pos, 0), 3)
}

pub fn sponge(ins: &[Bn254], tag: u64) -> Bn254 {
    let mut s = [Bn254::ZERO, Bn254::ZERO, Bn254::from_u64(tag)];
    for pair in ins.chunks(2) {
        s[0] += pair[0];
        if pair.len() > 1 {
            s[1] += pair[1];
        }
        p3_poseidon2_ffi::perm().permute_mut(&mut s);
    }
    s[0]
}

/// Little-endian bits of the canonical representative.
pub fn bits(x: Bn254, n: usize) -> Vec<bool> {
    let v = x.as_canonical_biguint();
    (0..n).map(|j| v.bit(j as u64)).collect()
}

pub fn chunks(h: Bn254) -> [usize; DIM] {
    let b = bits(h, DIM * W_BITS);
    core::array::from_fn(|i| (0..W_BITS).map(|t| (b[i * W_BITS + t] as usize) << t).sum())
}

// ---- instance ----
#[derive(Clone, Debug)]
pub struct Signer {
    pub param: Bn254,
    pub root: Bn254,
    pub rho: Bn254,
    pub revealed: [Bn254; DIM],
    pub path: [Bn254; HGT],
}

#[derive(Clone, Debug)]
pub struct Instance {
    pub epoch: u64,
    pub msg: [Bn254; MSG_ELEMS],
    pub signers: Vec<Signer>,
}

fn from_hex(h: &str) -> Bn254 {
    let v = BigUint::parse_bytes(h.as_bytes(), 16).expect("bad hex field element");
    assert!(v < Bn254::order(), "field element not canonical");
    Bn254::from_biguint(v).unwrap()
}

pub fn load_instance(path: &str) -> Instance {
    let text = std::fs::read_to_string(path).unwrap_or_else(|e| panic!("cannot read {path}: {e}"));
    let (mut epoch, mut k, mut msg, mut signers) = (0u64, 0usize, Vec::new(), Vec::new());
    for line in text.lines() {
        let mut w = line.split_whitespace();
        match w.next() {
            Some("params") => {
                let p: Vec<usize> = w.map(|x| x.parse().unwrap()).collect();
                assert_eq!(p, [W_BITS, DIM, HGT, MSG_ELEMS], "instance params do not match this build");
            }
            Some("k") => k = w.next().unwrap().parse().unwrap(),
            Some("epoch") => epoch = w.next().unwrap().parse().unwrap(),
            Some("msg") => msg = w.map(from_hex).collect(),
            Some("signer") => {
                let f: Vec<Bn254> = w.map(from_hex).collect();
                assert_eq!(f.len(), 3 + DIM + HGT, "malformed signer line");
                signers.push(Signer {
                    param: f[0],
                    root: f[1],
                    rho: f[2],
                    revealed: core::array::from_fn(|i| f[3 + i]),
                    path: core::array::from_fn(|l| f[3 + DIM + l]),
                });
            }
            _ => {}
        }
    }
    assert!(k > 0 && signers.len() == k && msg.len() == MSG_ELEMS, "instance file is incomplete");
    assert!(epoch < (1 << HGT));
    Instance { epoch, msg: [msg[0], msg[1]], signers }
}

/// Encoding hash of one signer: H(param, rho, m_0, m_1).
pub fn enc_hash(inst: &Instance, s: &Signer) -> Bn254 {
    sponge(&[s.param, s.rho, inst.msg[0], inst.msg[1]], tag_enc(inst.epoch, 4))
}

/// Chain ends of one signer (walk each revealed value from position x_i to BASE-1).
pub fn chain_ends(inst: &Instance, s: &Signer) -> [Bn254; DIM] {
    let x = chunks(enc_hash(inst, s));
    core::array::from_fn(|i| {
        let mut v = s.revealed[i];
        for pos in x[i] + 1..=MAXSTEPS {
            v = sponge(&[s.param, v], tag_chain(inst.epoch, i as u64, pos as u64));
        }
        v
    })
}

/// Recompute the Merkle root from a signature (generalized_xmss.hpp::verify, natively).
pub fn native_root(inst: &Instance, s: &Signer) -> Bn254 {
    let ends = chain_ends(inst, s);
    let mut leaf_in = vec![s.param];
    leaf_in.extend_from_slice(&ends);
    let mut cur = sponge(&leaf_in, tag_leaf(inst.epoch, leaf_in.len()));
    for l in 0..HGT {
        let tag = tag_node(l as u64 + 1, inst.epoch >> (l + 1));
        cur = if (inst.epoch >> l) & 1 == 1 {
            sponge(&[s.param, s.path[l], cur], tag)
        } else {
            sponge(&[s.param, cur, s.path[l]], tag)
        };
    }
    cur
}
