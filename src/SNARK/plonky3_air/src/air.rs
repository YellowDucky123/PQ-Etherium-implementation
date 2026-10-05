//! AIR for "k Generalized-XMSS signatures on one message all verify", over BN254 with
//! the Poseidon2 tweak hash. Same relation as the Aurora R1CS
//! (SNARK/aurora/aggregate_aurora_verify.cpp):
//!
//!   encode   h = H(param, rho, m0, m1); strict 254-bit decomposition; x_i = chunk i of h
//!   chains   walk revealed value i from position x_i to BASE-1 (tweak carries x_i + j)
//!   merkle   leaf = H(param, chain ends); fold the path in the epoch's bit order; == root
//!
//! LAYOUT. Each row is ONE Poseidon2 permutation (sponge rate 2, capacity 1). A signer is a
//! block of ROWS_PER_SIGNER rows with a fixed schedule (enc 2, chains DIM*MAXSTEPS, leaf 3,
//! nodes 2*HGT). The schedule (which values are absorbed, sponge continuation, tags, row
//! roles) is in PREPROCESSED columns, committed once in the verifying key. Epoch is part
//! of that schedule (tags + Merkle orientation), as in the Aurora circuit where it is a
//! build-time constant.
//!
//! Per-signer values that several rows need (param, rho, message, revealed values, chain
//! ends, path siblings, chunk bits) are REGISTERS: columns held constant across a block.
//! HOLD carries the last finished hash output to later rows.
//!
//! Public values: [m0, m1, param_0, root_0, ..., param_{k-1}, root_{k-1}].
//! All constraints have degree <= 3.

use p3_air::{Air, AirBuilder, AirBuilderWithPublicValues, BaseAir, BaseAirWithPublicValues, PairBuilder};
use p3_bn254::Bn254;
use p3_field::{Field, PrimeCharacteristicRing};
use p3_matrix::Matrix;
use p3_matrix::dense::RowMajorMatrix;

use crate::native::{self, BASE, DIM, HGT, Instance, MAXSTEPS, W_BITS};
use crate::poseidon2::{self, T};

pub const FBITS: usize = 254;
const LEAF_IN: usize = 1 + DIM;
const LEAF_ROWS: usize = LEAF_IN.div_ceil(2);
pub const ROWS_PER_SIGNER: usize = 2 + DIM * MAXSTEPS + LEAF_ROWS + 2 * HGT;

// ---------------- main trace columns ----------------
const IN: usize = 0; // 3
const SB: usize = IN + T; // 160
const OUT0: usize = SB + poseidon2::COLS; // committed copy of output[0]
const A: usize = OUT0 + 1; // 2 absorbed values
const REG: usize = A + 2;
// registers
const R_P: usize = 0;
const R_RHO: usize = 1;
const R_M: usize = 2; // 2
const R_S: usize = R_M + 2; // DIM revealed chain values
const R_E: usize = R_S + DIM; // DIM chain ends
const R_SIB: usize = R_E + DIM; // HGT siblings
const NREG: usize = R_SIB + HGT;
const HOLD: usize = REG + NREG;
const XB: usize = HOLD + 1; // DIM*W_BITS chunk bits (register)
const CB: usize = XB + DIM * W_BITS; // W_BITS bits of the current chain's chunk
const IND: usize = CB + W_BITS; // BASE one-hot [x_i == v]
const ACC: usize = IND + BASE; // chain-end accumulator
const BITS: usize = ACC + 1; // FBITS
const EQ: usize = BITS + FBITS; // one column per set bit of p (alias check)

// sources an absorb slot can read: registers, then HOLD
const NSRC: usize = NREG + 1;
const SRC_HOLD: usize = NREG;

// ---------------- preprocessed columns ----------------
const P_CARRY: usize = 0;
const P_TAG: usize = 1;
const P_CHSEL: usize = 2; // DIM
const P_STEP: usize = P_CHSEL + DIM; // MAXSTEPS
const P_ENDSEL: usize = P_STEP + MAXSTEPS; // DIM
const P_ENDHASH: usize = P_ENDSEL + DIM;
const P_SAME: usize = P_ENDHASH + 1;
const P_BITSEL: usize = P_SAME + 1;
const P_SRC0: usize = P_BITSEL + 1; // NSRC
const P_SRC1: usize = P_SRC0 + NSRC; // NSRC
const P_FIRST: usize = P_SRC1 + NSRC; // k, then LAST: k

#[derive(Clone, Copy, Default)]
enum Src {
    #[default]
    Zero,
    Reg(usize),
    Hold,
}

#[derive(Clone, Default)]
struct RowSpec {
    carry: bool,
    tag: u64,
    chain: Option<(usize, usize)>, // (chain index i, step j in 1..=MAXSTEPS)
    src: [Src; 2],
    endhash: bool,
    bitsel: bool,
}

/// The fixed schedule of one signer block.
fn block_schedule(epoch: u64) -> Vec<RowSpec> {
    let mut rows = Vec::with_capacity(ROWS_PER_SIGNER);
    // a hash over `ins` (absorbed two at a time) with `tag`
    let mut hash = |ins: &[Src], tag: u64, chain: Option<(usize, usize)>, bitsel: bool| {
        let n = ins.len().div_ceil(2);
        for r in 0..n {
            rows.push(RowSpec {
                carry: r > 0,
                tag: if r == 0 { tag } else { 0 },
                chain,
                src: [ins[2 * r], ins.get(2 * r + 1).copied().unwrap_or(Src::Zero)],
                endhash: r == n - 1,
                bitsel: bitsel && r == n - 1,
            });
        }
    };
    let p = Src::Reg(R_P);
    hash(&[p, Src::Reg(R_RHO), Src::Reg(R_M), Src::Reg(R_M + 1)], native::tag_enc(epoch, 4), None, true);
    for i in 0..DIM {
        for j in 1..=MAXSTEPS {
            let prev = if j == 1 { Src::Reg(R_S + i) } else { Src::Hold };
            // tag constant has c = j; the AIR adds x_i (pos = x_i + j)
            hash(&[p, prev], native::tag_chain(epoch, i as u64, j as u64), Some((i, j)), false);
        }
    }
    let mut leaf = vec![p];
    leaf.extend((0..DIM).map(|i| Src::Reg(R_E + i)));
    hash(&leaf, native::tag_leaf(epoch, LEAF_IN), None, false);
    for l in 0..HGT {
        let sib = Src::Reg(R_SIB + l);
        let ins = if (epoch >> l) & 1 == 1 { [p, sib, Src::Hold] } else { [p, Src::Hold, sib] };
        hash(&ins, native::tag_node(l as u64 + 1, epoch >> (l + 1)), None, false);
    }
    assert_eq!(rows.len(), ROWS_PER_SIGNER);
    rows
}

pub struct XmssAggAir {
    pub k: usize,
    pub epoch: u64,
    pub height: usize,
    rc: Vec<[Bn254; T]>,
    p_bits: Vec<bool>, // bits of the modulus, LE
    n_eq: usize,
    pow2: Vec<Bn254>,
}

impl XmssAggAir {
    pub fn new(k: usize, epoch: u64) -> Self {
        let p_bits = modulus_bits();
        let n_eq = p_bits.iter().filter(|b| **b).count();
        let mut pow2 = vec![Bn254::ONE; FBITS];
        for j in 1..FBITS {
            pow2[j] = pow2[j - 1].double();
        }
        let height = (k * ROWS_PER_SIGNER).next_power_of_two();
        Self { k, epoch, height, rc: poseidon2::round_constants(), p_bits, n_eq, pow2 }
    }

    fn p_width(&self) -> usize {
        P_FIRST + 2 * self.k
    }

    pub fn public_values(inst: &Instance) -> Vec<Bn254> {
        let mut pv = inst.msg.to_vec();
        for s in &inst.signers {
            pv.push(s.param);
            pv.push(s.root);
        }
        pv
    }

    /// Fill the main trace from real signatures. Native values (chunks, chain ends) are
    /// recomputed with Plonky3's permutation; each row's permutation is checked against it.
    pub fn generate_trace(&self, inst: &Instance) -> RowMajorMatrix<Bn254> {
        assert_eq!(inst.signers.len(), self.k);
        let w = self.width();
        let mut vals = vec![Bn254::ZERO; self.height * w];
        let sched = block_schedule(self.epoch);
        let mut prev_out = [Bn254::ZERO; T];
        let mut hold = Bn254::ZERO;
        let mut prev_endhash = false;
        let mut prev_out0 = Bn254::ZERO;

        for r in 0..self.height {
            let acc_prev = if r > 0 { vals[(r - 1) * w + ACC] } else { Bn254::ZERO };
            let row = &mut vals[r * w..(r + 1) * w];
            // HOLD follows the transition rule from the previous row
            if r > 0 && prev_endhash {
                hold = prev_out0;
            }
            row[HOLD] = hold;

            let (spec, signer) = if r < self.k * ROWS_PER_SIGNER {
                (sched[r % ROWS_PER_SIGNER].clone(), Some(&inst.signers[r / ROWS_PER_SIGNER]))
            } else {
                (RowSpec::default(), None)
            };

            let mut x = [0usize; DIM];
            if let Some(s) = signer {
                let ends = native::chain_ends(inst, s);
                x = native::chunks(native::enc_hash(inst, s));
                let mut regs = vec![Bn254::ZERO; NREG];
                regs[R_P] = s.param;
                regs[R_RHO] = s.rho;
                regs[R_M] = inst.msg[0];
                regs[R_M + 1] = inst.msg[1];
                for i in 0..DIM {
                    regs[R_S + i] = s.revealed[i];
                    regs[R_E + i] = ends[i];
                }
                for l in 0..HGT {
                    regs[R_SIB + l] = s.path[l];
                }
                row[REG..REG + NREG].copy_from_slice(&regs);
                for i in 0..DIM {
                    for t in 0..W_BITS {
                        row[XB + i * W_BITS + t] = Bn254::from_bool((x[i] >> t) & 1 == 1);
                    }
                }
            }

            let read = |src: Src, row: &[Bn254]| match src {
                Src::Zero => Bn254::ZERO,
                Src::Reg(i) => row[REG + i],
                Src::Hold => row[HOLD],
            };
            let a = [read(spec.src[0], row), read(spec.src[1], row)];
            row[A] = a[0];
            row[A + 1] = a[1];

            let input = if spec.carry {
                [prev_out[0] + a[0], prev_out[1] + a[1], prev_out[2]]
            } else {
                let xi = spec.chain.map_or(0, |(i, _)| x[i] as u64);
                [a[0], a[1], Bn254::from_u64(spec.tag + xi)]
            };
            row[IN..IN + T].copy_from_slice(&input);
            let out = poseidon2::permute_record(input, &self.rc, &mut row[SB..SB + poseidon2::COLS]);
            row[OUT0] = out[0];

            if let Some((i, j)) = spec.chain {
                let xi = x[i];
                for t in 0..W_BITS {
                    row[CB + t] = Bn254::from_bool((xi >> t) & 1 == 1);
                }
                row[IND + xi] = Bn254::ONE;
                let ind = |v: usize| if v == xi { Bn254::ONE } else { Bn254::ZERO };
                row[ACC] = if j == 1 {
                    ind(MAXSTEPS) * a[1] + ind(MAXSTEPS - 1) * out[0]
                } else {
                    acc_prev + ind(MAXSTEPS - j) * out[0]
                };
                if j == MAXSTEPS {
                    assert_eq!(row[ACC], row[REG + R_E + i], "chain end mismatch");
                }
            }

            if spec.bitsel {
                let b = native::bits(out[0], FBITS);
                let mut eq = Bn254::ONE;
                let mut k = 0;
                for jj in (0..FBITS).rev() {
                    row[BITS + jj] = Bn254::from_bool(b[jj]);
                    if self.p_bits[jj] {
                        eq *= row[BITS + jj];
                        row[EQ + k] = eq;
                        k += 1;
                    }
                }
            }

            if let Some(s) = signer {
                if r % ROWS_PER_SIGNER == ROWS_PER_SIGNER - 1 {
                    assert_eq!(out[0], s.root, "signer's recomputed root != public key");
                }
            }

            prev_out = out;
            prev_out0 = out[0];
            prev_endhash = spec.endhash;
        }
        RowMajorMatrix::new(vals, w)
    }
}

impl BaseAir<Bn254> for XmssAggAir {
    fn width(&self) -> usize {
        EQ + self.n_eq
    }

    fn preprocessed_trace(&self) -> Option<RowMajorMatrix<Bn254>> {
        let w = self.p_width();
        let mut vals = vec![Bn254::ZERO; self.height * w];
        let sched = block_schedule(self.epoch);
        for s in 0..self.k {
            for (q, spec) in sched.iter().enumerate() {
                let row = &mut vals[(s * ROWS_PER_SIGNER + q) * w..][..w];
                row[P_CARRY] = Bn254::from_bool(spec.carry);
                row[P_TAG] = Bn254::from_u64(spec.tag);
                if let Some((i, j)) = spec.chain {
                    row[P_CHSEL + i] = Bn254::ONE;
                    row[P_STEP + j - 1] = Bn254::ONE;
                    if j == MAXSTEPS {
                        row[P_ENDSEL + i] = Bn254::ONE;
                    }
                }
                row[P_ENDHASH] = Bn254::from_bool(spec.endhash);
                row[P_SAME] = Bn254::from_bool(q + 1 < ROWS_PER_SIGNER);
                row[P_BITSEL] = Bn254::from_bool(spec.bitsel);
                for (slot, base) in [(0, P_SRC0), (1, P_SRC1)] {
                    match spec.src[slot] {
                        Src::Zero => {}
                        Src::Reg(i) => row[base + i] = Bn254::ONE,
                        Src::Hold => row[base + SRC_HOLD] = Bn254::ONE,
                    }
                }
                if q == 0 {
                    row[P_FIRST + s] = Bn254::ONE;
                }
                if q == ROWS_PER_SIGNER - 1 {
                    row[P_FIRST + self.k + s] = Bn254::ONE;
                }
            }
        }
        Some(RowMajorMatrix::new(vals, w))
    }
}

impl BaseAirWithPublicValues<Bn254> for XmssAggAir {
    fn num_public_values(&self) -> usize {
        2 + 2 * self.k
    }
}

impl<AB: AirBuilderWithPublicValues<F = Bn254> + PairBuilder> Air<AB> for XmssAggAir {
    fn eval(&self, builder: &mut AB) {
        let main = builder.main();
        let local: Vec<AB::Var> = main.row_slice(0).unwrap().to_vec();
        let next: Vec<AB::Var> = main.row_slice(1).unwrap().to_vec();
        let prep = builder.preprocessed();
        let pl: Vec<AB::Var> = prep.row_slice(0).unwrap().to_vec();
        let pn: Vec<AB::Var> = prep.row_slice(1).unwrap().to_vec();
        let pv: Vec<AB::Expr> = builder.public_values().iter().map(|&v| v.into()).collect();

        let l = |i: usize| -> AB::Expr { local[i].clone().into() };
        let n = |i: usize| -> AB::Expr { next[i].clone().into() };
        let p = |i: usize| -> AB::Expr { pl[i].clone().into() };
        let q = |i: usize| -> AB::Expr { pn[i].clone().into() };
        let c = |f: Bn254| -> AB::Expr { AB::Expr::from(f) };
        let one = || AB::Expr::ONE;

        // ---- the permutation on this row ----
        let out = poseidon2::eval(builder, [l(IN), l(IN + 1), l(IN + 2)], &local[SB..SB + poseidon2::COLS], &self.rc);
        builder.assert_eq(l(OUT0), out[0].clone());

        // ---- sponge input ----
        let ischain = (0..DIM).fold(AB::Expr::ZERO, |acc, i| acc + p(P_CHSEL + i));
        let x = (0..W_BITS).fold(AB::Expr::ZERO, |acc, t| acc + l(CB + t) * c(Bn254::from_u64(1 << t)));
        let fresh = one() - p(P_CARRY);
        builder.assert_zero(fresh.clone() * (l(IN) - l(A)));
        builder.assert_zero(fresh.clone() * (l(IN + 1) - l(A + 1)));
        builder.assert_zero(fresh * (l(IN + 2) - p(P_TAG) - ischain.clone() * x));
        {
            let mut t = builder.when_transition();
            t.assert_zero(q(P_CARRY) * (n(IN) - out[0].clone() - n(A)));
            t.assert_zero(q(P_CARRY) * (n(IN + 1) - out[1].clone() - n(A + 1)));
            t.assert_zero(q(P_CARRY) * (n(IN + 2) - out[2].clone()));
        }

        // ---- absorbed values come from the scheduled source ----
        let src = |s: usize| if s == SRC_HOLD { l(HOLD) } else { l(REG + s) };
        for (slot, base) in [(0, P_SRC0), (1, P_SRC1)] {
            let sel = (0..NSRC).fold(AB::Expr::ZERO, |acc, s| acc + p(base + s) * src(s));
            builder.assert_eq(l(A + slot), sel);
        }

        // ---- registers are constant within a block; HOLD latches finished hashes ----
        {
            let mut t = builder.when_transition();
            for r in (REG..REG + NREG).chain(XB..XB + DIM * W_BITS) {
                t.assert_zero(p(P_SAME) * (n(r) - l(r)));
            }
            t.assert_zero(n(HOLD) - p(P_ENDHASH) * l(OUT0) - (one() - p(P_ENDHASH)) * l(HOLD));
        }

        // ---- encode: strict decomposition of the enc hash, chunk bits -> registers ----
        let bitsel = p(P_BITSEL);
        for j in 0..FBITS {
            builder.assert_bool(l(BITS + j));
        }
        let recomp = (0..FBITS).fold(AB::Expr::ZERO, |acc, j| acc + l(BITS + j) * c(self.pow2[j]));
        builder.assert_zero(bitsel.clone() * (recomp - l(OUT0)));
        // bits < p: MSB-first prefix comparison against the constant modulus
        let mut eq = one();
        let mut k = 0;
        for j in (0..FBITS).rev() {
            if self.p_bits[j] {
                builder.assert_zero(bitsel.clone() * (l(EQ + k) - eq.clone() * l(BITS + j)));
                eq = l(EQ + k);
                k += 1;
            } else {
                builder.assert_zero(bitsel.clone() * eq.clone() * l(BITS + j));
            }
        }
        builder.assert_zero(bitsel.clone() * eq);
        for m in 0..DIM * W_BITS {
            builder.assert_zero(bitsel.clone() * (l(XB + m) - l(BITS + m)));
        }

        // ---- chains: current chunk bits, one-hot indicator, chain-end accumulator ----
        for t in 0..W_BITS {
            let sel = (0..DIM).fold(AB::Expr::ZERO, |acc, i| acc + p(P_CHSEL + i) * l(XB + i * W_BITS + t));
            builder.assert_eq(l(CB + t), sel);
        }
        for v in 0..BASE {
            let prod = (0..W_BITS).fold(one(), |acc, t| {
                acc * if (v >> t) & 1 == 1 { l(CB + t) } else { one() - l(CB + t) }
            });
            builder.assert_zero(ischain.clone() * (l(IND + v) - prod));
        }
        // chain_end = node[MAXSTEPS - x]; node[0] = revealed value (A1 of step 1), node[j] = out of step j
        builder.assert_zero(
            p(P_STEP) * (l(ACC) - l(IND + MAXSTEPS) * l(A + 1) - l(IND + MAXSTEPS - 1) * l(OUT0)),
        );
        {
            let mut t = builder.when_transition();
            for j in 2..=MAXSTEPS {
                t.assert_zero(q(P_STEP + j - 1) * (n(ACC) - l(ACC) - n(IND + MAXSTEPS - j) * n(OUT0)));
            }
        }
        for i in 0..DIM {
            builder.assert_zero(p(P_ENDSEL + i) * (l(REG + R_E + i) - l(ACC)));
        }

        // ---- statement: message, each signer's param (first row) and root (last row) ----
        let first_any = (0..self.k).fold(AB::Expr::ZERO, |acc, s| acc + p(P_FIRST + s));
        builder.assert_zero(first_any.clone() * (l(REG + R_M) - pv[0].clone()));
        builder.assert_zero(first_any * (l(REG + R_M + 1) - pv[1].clone()));
        for s in 0..self.k {
            builder.assert_zero(p(P_FIRST + s) * (l(REG + R_P) - pv[2 + 2 * s].clone()));
            builder.assert_zero(p(P_FIRST + self.k + s) * (l(OUT0) - pv[3 + 2 * s].clone()));
        }
    }
}

fn modulus_bits() -> Vec<bool> {
    let v = Bn254::order();
    (0..FBITS).map(|j| v.bit(j as u64)).collect()
}

/// Main-trace column of revealed-chain-value register i (for tamper tests).
pub fn col_reg_s(i: usize) -> usize {
    REG + R_S + i
}
