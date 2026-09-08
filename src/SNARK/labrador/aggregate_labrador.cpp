/*  aggregate_labrador.cpp
 *
 *  Aggregate k Generalized-XMSS multi-signatures into ONE compact,
 *  post-quantum (Module-SIS) proof using LaBRADOR over ICICLE.
 *
 *  This is the LaBRADOR counterpart to aggregate_aurora.cpp. Where Aurora is a
 *  hash/FRI-based zk-STARK, LaBRADOR is a *lattice* proof: transparent, plausibly
 *  post-quantum under Module-SIS, and far more compact after recursion.
 *
 *  Flow:
 *    1. Flatten k signatures (bytes) -> bit stream (each bit in {0,1}).
 *    2. Pack the bits as the coefficients of a low-norm witness S in Rq^{r*n}.
 *       LaBRADOR requires a *small-norm* witness, so the bit-decomposition of
 *       the signatures is exactly the right representation.
 *    3. Build LaBRADOR EQ + ConstZero constraints satisfied by S, and prove
 *       knowledge of that low-norm witness. Then verify.
 *
 *  Honesty note (see src/SNARK/README_PROOFSYSTEMS.md):
 *    The proof binds to the real signature witness and demonstrates the whole
 *    LaBRADOR prove/verify pipeline end-to-end. The constraints are the generic
 *    (committed) relation LaBRADOR proves natively; full arithmetization of
 *    XMSS verification (hash chains + Merkle path) into LaBRADOR's inner-product
 *    constraint form is the remaining research work.
 */

#include "labrador.h"
#include "icicle/runtime.h"
#include "types.h"
#include "utils.h"
#include "prover.h"
#include "verifier.h"
#include "shared.h"
#include "test_helpers.h"
#include "examples_utils.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <string>

using namespace icicle::labrador;

// Pack a flat bit stream into r*n low-norm Rq polynomials (coefficients in {0,1}).
static std::vector<Rq> witness_from_bits(const std::vector<uint8_t>& bits, size_t r, size_t n)
{
  const size_t d = Rq::d;
  std::vector<Rq> S(r * n);
  size_t pos = 0;
  for (auto& poly : S) {
    for (size_t i = 0; i < d; ++i) {
      uint64_t v = (pos < bits.size()) ? bits[pos] : 0;
      poly.values[i] = Zq::from(v);
      ++pos;
    }
  }
  return S;
}

int main(int argc, char* argv[])
{
  ICICLE_LOG_INFO << "LaBRADOR multi-signature aggregation";
  try_load_and_set_backend_device(argc, argv);

  // ---- fabricate k signatures of realistic size (same shape as the Aurora demo) ----
  const size_t k         = 8;      // aggregate 8 signers
  const size_t sig_bytes = 2144;   // Winternitz signature ~ 2 KB each

  std::vector<uint8_t> flat_bits;
  flat_bits.reserve(k * sig_bytes * 8);
  for (size_t s = 0; s < k; ++s)
    for (size_t i = 0; i < sig_bytes; ++i) {
      uint8_t byte = uint8_t((s * 17 + i * 5) & 0xff);
      for (int b = 0; b < 8; ++b) flat_bits.push_back((byte >> b) & 1);
    }

  // ---- choose problem size (r * n * d coefficients) big enough to hold the witness ----
  constexpr size_t d = Rq::d;
  const size_t n = 1 << 8;   // vector dimension
  const size_t r = 1 << 4;   // number of witness vectors
  const size_t capacity = r * n * d;
  std::cout << "Aggregating k=" << k << " signatures (" << k * sig_bytes
            << " bytes = " << flat_bits.size() << " bits) into a witness of "
            << capacity << " coefficients (r=" << r << ", n=" << n << ", d=" << d << ").\n";
  if (flat_bits.size() > capacity)
    std::cout << "NOTE: witness larger than capacity; extra bits truncated for this demo size.\n";

  std::vector<Rq> S = witness_from_bits(flat_bits, r, n);

  // ---- constraints satisfied by S (LaBRADOR's native committed relation) ----
  const size_t num_eq_const = 10;
  const size_t num_cz_const = 10;
  auto eq_inst = create_rand_eq_inst(n, r, S, num_eq_const);
  std::cout << "Created EQ constraints\n";
  auto const_zero_inst = create_rand_const_zero_inst(n, r, S, num_cz_const);
  std::cout << "Created ConstZero constraints\n";

  // ---- protocol parameters ----
  const int64_t max_value = 1; // witness coefficients are bits -> norm bound uses 1
  double beta = sqrt((double)max_value * n * r * d);
  uint32_t base0 = calc_base0(r, OP_NORM_BOUND, beta);

  auto now = std::chrono::system_clock::now();
  std::string ajtai_seed_str = std::to_string(
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

  LabradorParam param{
    r, n,
    {reinterpret_cast<const std::byte*>(ajtai_seed_str.data()),
     reinterpret_cast<const std::byte*>(ajtai_seed_str.data()) + ajtai_seed_str.size()},
    secure_msis_rank(), secure_msis_rank(), secure_msis_rank(),
    base0, base0, base0, beta};

  LabradorInstance lab_inst{param};
  lab_inst.add_equality_constraint(eq_inst);
  lab_inst.add_const_zero_constraint(const_zero_inst);

  std::string oracle_seed = "ORACLE_SEED";
  // NUM_REC (recursion depth) can be overridden as the 2nd CLI arg after the device.
  size_t NUM_REC = 6;
  if (argc >= 3) NUM_REC = std::stoul(argv[2]);
  std::cout << "NUM_REC = " << NUM_REC << "\n";

  // ---- prove ----
  auto t0 = std::chrono::high_resolution_clock::now();
  LabradorProver prover{lab_inst, S,
                        reinterpret_cast<const std::byte*>(oracle_seed.data()),
                        oracle_seed.size(), NUM_REC};
  auto [trs, final_proof] = prover.prove();
  auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << "[labrador] prove time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";

  // TOTAL proof = per-recursion transcript messages + final base-case proof.
  size_t transcript_bytes = 0;
  for (auto& t : trs) transcript_bytes += t.proof_size();
  size_t total_bytes = transcript_bytes + final_proof.size();
  std::cout << "[labrador] recursion transcripts: " << trs.size()
            << ", transcript bytes: " << transcript_bytes
            << ", base-case proof: " << final_proof.size() << " bytes\n";
  std::cout << "[labrador] TOTAL proof size: " << total_bytes
            << " bytes (" << (total_bytes / 1024.0) << " KB)\n";

  std::vector<BaseProverMessages> prover_msgs;
  for (const auto& t : trs) prover_msgs.push_back(t.prover_msg);

  // ---- verify ----
  auto t2 = std::chrono::high_resolution_clock::now();
  LabradorVerifier verifier{lab_inst, prover_msgs, final_proof,
                            reinterpret_cast<const std::byte*>(oracle_seed.data()),
                            oracle_seed.size(), NUM_REC};
  bool ok = verifier.verify();
  auto t3 = std::chrono::high_resolution_clock::now();
  std::cout << "[labrador] verify time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << " ms\n";
  std::cout << "[labrador] VERIFICATION: " << (ok ? "SUCCESS" : "FAILED") << "\n";
  return ok ? 0 : 1;
}
