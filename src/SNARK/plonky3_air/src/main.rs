//! Plonky3 aggregation of k Generalized-XMSS signatures (Poseidon2-BN254 tweak hash).
//!
//! Usage: p3_xmss_agg <instance-file> [num_queries=56] [log_blowup=2]
//!
//! The instance is written by ../export_p2_sigs from REAL signatures of the C++ core.
//! Defaults match the Aurora harness's achieved security: rate 1/4, 56 queries
//! = 112 bits under the ethSTARK conjecture (Aurora reports "achieved 112.0" heuristic).

mod air;
mod config;
mod native;
mod poseidon2;

use std::time::Instant;

use p3_bn254::Bn254;
use p3_field::PrimeCharacteristicRing;
use p3_matrix::Matrix;
use p3_uni_stark::{prove_with_preprocessed, setup_preprocessed, verify_with_preprocessed};

use air::XmssAggAir;
use config::{Params, make_config};

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("usage: {} <instance-file> [num_queries=56] [log_blowup=2]", args[0]);
        std::process::exit(2);
    }
    let params = Params {
        num_queries: args.get(2).map_or(56, |s| s.parse().unwrap()),
        log_blowup: args.get(3).map_or(2, |s| s.parse().unwrap()),
        pow_bits: 0,
    };

    let inst = native::load_instance(&args[1]);
    let k = inst.signers.len();
    for (s, sg) in inst.signers.iter().enumerate() {
        assert_eq!(native::native_root(&inst, sg), sg.root, "signer {s}: core signature does not verify under the AIR's hash");
    }
    println!("[plonky3] loaded {k} REAL core signatures (epoch {}); native roots == public keys", inst.epoch);

    let air = XmssAggAir::new(k, inst.epoch);
    let pv = XmssAggAir::public_values(&inst);

    let tb = Instant::now();
    let trace = air.generate_trace(&inst);
    let build_ms = tb.elapsed().as_millis();
    let prep_width = p3_air::BaseAir::<Bn254>::preprocessed_trace(&air).unwrap().width();
    let max_deg = p3_uni_stark::get_max_constraint_degree::<Bn254, _>(&air, prep_width, pv.len());
    let log_q = p3_uni_stark::get_log_num_quotient_chunks::<Bn254, _>(&air, prep_width, pv.len(), 0);
    println!(
        "[plonky3] AIR: {} rows ({} used, {}/signer) x {} main cols + {} preprocessed cols; {} constraints (max degree {}), {} quotient chunks",
        trace.height(),
        k * air::ROWS_PER_SIGNER,
        air::ROWS_PER_SIGNER,
        trace.width(),
        prep_width,
        p3_uni_stark::get_symbolic_constraints::<Bn254, _>(&air, prep_width, pv.len()).len(),
        max_deg,
        1usize << log_q
    );
    println!(
        "[plonky3] FRI: log_blowup={} queries={} pow={} -> {} bits (ethSTARK conjecture)",
        params.log_blowup,
        params.num_queries,
        params.pow_bits,
        params.log_blowup * params.num_queries + params.pow_bits
    );

    let config = make_config(&params);
    let degree_bits = trace.height().trailing_zeros() as usize;

    let ts = Instant::now();
    let (prep_prover, prep_vk) = setup_preprocessed(&config, &air, degree_bits).expect("preprocessed");
    let setup_ms = ts.elapsed().as_millis();

    let tp = Instant::now();
    let proof = prove_with_preprocessed(&config, &air, trace.clone(), &pv, Some(&prep_prover));
    let prove_ms = tp.elapsed().as_millis();
    let proof_bytes = postcard::to_allocvec(&proof).expect("serialize").len();
    println!("[plonky3] proof size: {proof_bytes} bytes | prove {prove_ms} ms (setup/preprocessed commit {setup_ms} ms, trace gen {build_ms} ms)");

    let tv = Instant::now();
    let res = verify_with_preprocessed(&config, &air, &proof, &pv, Some(&prep_vk));
    let verify_ms = tv.elapsed().as_millis();
    let ok = res.is_ok();
    println!("[plonky3] verify {verify_ms} ms | VERIFICATION: {}", if ok { "SUCCESS".into() } else { format!("FAILED {res:?}") });

    // Soundness spot checks: a corrupted witness or a wrong public key must not verify.
    {
        let mut bad = trace.clone();
        let w = bad.width();
        // signer 0's chain-0 step-1 row (row 2): bump the revealed value register
        bad.values[2 * w + air_reg_s0()] += Bn254::ONE;
        let bad_proof = prove_with_preprocessed(&config, &air, bad, &pv, Some(&prep_prover));
        let rejected = verify_with_preprocessed(&config, &air, &bad_proof, &pv, Some(&prep_vk)).is_err();
        println!("[plonky3] tamper check (corrupt a revealed chain value): {}", if rejected { "rejected" } else { "STILL ACCEPTED - BUG!" });

        let mut bad_pv = pv.clone();
        bad_pv[3] += Bn254::ONE; // signer 0's root
        let rejected = verify_with_preprocessed(&config, &air, &proof, &bad_pv, Some(&prep_vk)).is_err();
        println!("[plonky3] wrong public key (root of signer 0 + 1): {}", if rejected { "rejected" } else { "STILL ACCEPTED - BUG!" });
    }

    println!(
        "RESULT system=plonky3 k={k} rows={} cols={} queries={} log_blowup={} proof_bytes={proof_bytes} build_ms={build_ms} setup_ms={setup_ms} prove_ms={prove_ms} verify_ms={verify_ms} ok={}",
        trace.height(),
        trace.width(),
        params.num_queries,
        params.log_blowup,
        ok as u8
    );
    std::process::exit(if ok { 0 } else { 1 });
}

fn air_reg_s0() -> usize {
    air::col_reg_s(0)
}
