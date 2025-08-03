#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "../generalized_xmss.hpp"
#include <random>
#include <iostream>

// ---- Dummy PRF ----
struct DummyPRF {
    struct Key {};
    struct Output {
        std::array<uint8_t, 32> bytes;
        auto into() const { return *this; }
    };

    static Key gen(std::mt19937&) { return {}; }
    static Output apply(const Key&, uint32_t, uint64_t) { return {}; }
};

template<>
struct std::hash<DummyPRF::Output> {
    size_t operator()(const DummyPRF::Output&) const { return 0; }
};

// ---- Dummy Encoding ----
struct DummyEncoding {
    using Parameter = int;
    using Randomness = int;
    using EncodingResult = std::vector<int>;

    static constexpr int DIMENSION = 4;
    static constexpr int BASE = 16;
    static constexpr int MAX_TRIES = 3;

    static Randomness rand(std::mt19937&) { return 42; }

    static std::optional<EncodingResult> encode(Parameter, const std::array<uint8_t, 32>&, Randomness, uint32_t) {
        return std::vector<int>{1, 2, 3, 4};
    }
};

// ---- Dummy Tweakable Hash ----
struct DummyTH {
    struct Domain {
        Domain() = default;
        Domain(const Domain&) = default;
        Domain& operator=(const Domain&) = default;
        Domain(Domain&&) = default;
        Domain& operator=(Domain&&) = default;
        ~Domain() = default;

        Domain(DummyPRF::Output const &out): bytes(out.bytes) {}
        std::array<uint8_t, 32> bytes {};
        bool operator==(const Domain&) const = default;
    };
    using Parameter = int;

    static Parameter rand_parameter(std::mt19937&) { return 0; }

    static Domain apply(Parameter, uint64_t, const std::vector<Domain>&) {
        return {};
    }

    static uint64_t tree_tweak(uint8_t, uint32_t) { return 0; }
};



// ---- Typedef for scheme ----
using XMSS = GeneralizedXMSSSignatureScheme<
    DummyPRF,
    DummyEncoding,
    DummyTH,
    3,              // LIFETIME = 8
    DummyHashTree
>;

TEST_CASE("XMSS generates and signs correctly") {
    std::mt19937 rng(12345);
    auto [pk, sk] = XMSS::gen(rng);

    std::array<uint8_t, 32> message{};
    for (uint8_t i = 0; i < 32; ++i) message[i] = i;

    auto sig_opt = XMSS::sign(rng, sk, 1, message);
    REQUIRE(sig_opt.has_value());

    auto sig = sig_opt.value();
    bool valid = XMSS::verify(pk, 1, message, sig);
    REQUIRE(valid);
}
