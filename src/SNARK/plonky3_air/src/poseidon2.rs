//! Poseidon2-BN254 (t=3, x^5, R_F=8, R_P=56) as AIR columns: one permutation per row.
//!
//! Every S-box y = x^5 is committed as two columns (c = x^3, y = c * x^2), so each S-box
//! costs two degree-3 constraints. Round constants and the linear layers are linear and
//! stay symbolic. The round-constant table and linear layers are the HorizenLabs ones, the
//! same that `p3_poseidon2_ffi::perm()` (Plonky3's `Poseidon2Bn254<3>`) uses; trace
//! generation checks every row against that permutation.

use p3_air::AirBuilder;
use p3_bn254::Bn254;
use p3_field::PrimeCharacteristicRing;
use p3_symmetric::Permutation;

pub const T: usize = 3;
pub const RF: usize = 8;
pub const RP: usize = 56;
pub const NSBOX: usize = RF * T + RP; // 80
pub const COLS: usize = 2 * NSBOX; // 160

pub fn round_constants() -> Vec<[Bn254; T]> {
    p3_poseidon2_ffi::round_constants()
}

fn mat_external<E: PrimeCharacteristicRing>(s: &mut [E; T]) {
    // circ(2,1,1)
    let sum = s[0].clone() + s[1].clone() + s[2].clone();
    for x in s.iter_mut() {
        *x = x.clone() + sum.clone();
    }
}

fn mat_internal<E: PrimeCharacteristicRing>(s: &mut [E; T]) {
    // 1 + diag(1,1,2)
    let sum = s[0].clone() + s[1].clone() + s[2].clone();
    s[0] = s[0].clone() + sum.clone();
    s[1] = s[1].clone() + sum.clone();
    s[2] = s[2].double() + sum;
}

/// Native permutation that also records the S-box columns into `cols` (len COLS).
pub fn permute_record(input: [Bn254; T], rc: &[[Bn254; T]], cols: &mut [Bn254]) -> [Bn254; T] {
    let mut s = input;
    let mut k = 0;
    let mut sbox = |x: Bn254, cols: &mut [Bn254]| {
        let c = x * x * x;
        let y = c * x * x;
        cols[2 * k] = c;
        cols[2 * k + 1] = y;
        k += 1;
        y
    };
    mat_external(&mut s);
    for r in 0..RF / 2 {
        for i in 0..T {
            s[i] = sbox(s[i] + rc[r][i], cols);
        }
        mat_external(&mut s);
    }
    for r in RF / 2..RF / 2 + RP {
        s[0] = sbox(s[0] + rc[r][0], cols);
        mat_internal(&mut s);
    }
    for r in RF / 2 + RP..RF + RP {
        for i in 0..T {
            s[i] = sbox(s[i] + rc[r][i], cols);
        }
        mat_external(&mut s);
    }
    let mut check = input;
    p3_poseidon2_ffi::perm().permute_mut(&mut check);
    assert_eq!(s, check, "AIR Poseidon2 trace != Plonky3 Poseidon2Bn254<3>");
    s
}

/// Constrain one permutation: `input` are the state expressions, `cols` the S-box columns.
/// Returns the output state as (degree-1) expressions.
pub fn eval<AB: AirBuilder<F = Bn254>>(
    builder: &mut AB,
    input: [AB::Expr; T],
    cols: &[AB::Var],
    rc: &[[Bn254; T]],
) -> [AB::Expr; T] {
    let mut s = input;
    let mut k = 0;
    let mut sbox = |x: AB::Expr, builder: &mut AB| -> AB::Expr {
        let c: AB::Expr = cols[2 * k].clone().into();
        let y: AB::Expr = cols[2 * k + 1].clone().into();
        builder.assert_zero(c.clone() - x.clone() * x.clone() * x.clone());
        builder.assert_zero(y.clone() - c * x.clone() * x);
        k += 1;
        y
    };
    mat_external(&mut s);
    for r in 0..RF / 2 {
        for i in 0..T {
            s[i] = sbox(s[i].clone() + AB::Expr::from(rc[r][i]), builder);
        }
        mat_external(&mut s);
    }
    for r in RF / 2..RF / 2 + RP {
        s[0] = sbox(s[0].clone() + AB::Expr::from(rc[r][0]), builder);
        mat_internal(&mut s);
    }
    for r in RF / 2 + RP..RF + RP {
        for i in 0..T {
            s[i] = sbox(s[i].clone() + AB::Expr::from(rc[r][i]), builder);
        }
        mat_external(&mut s);
    }
    s
}
