#include <iostream>
#include <vector>
#include "../inc_encoding.hpp"
#include "../symmetric/message_hash.hpp"
#include "../constraint.hpp"

/// Incomparable Encoding Scheme based on Target Sums,
/// implemented from a given message hash.
/// CHUNK_SIZE has to be 1,2,4, or 8.
/// TARGET_SUM determines how we set the target sum,
/// and has direct impact on the signer's running time,
/// or equivalently the success probability of this encoding scheme.
/// It is recommended to set it close to the expected sum, which is:
///
/// ```ignore
///     const MAX_CHUNK_VALUE: usize = MH::BASE - 1
///     const EXPECTED_SUM: usize = MH::DIMENSION * MAX_CHUNK_VALUE / 2
/// ```

// Target Sum Winternitz OTS overview
// We define a target sum T
// Only allow messages that result in a pre-defined
// sum of interim values

template <typename MH, size_t TARGET_SUM>
class TargetSumEncoding
{
private:
    MH message_hash;

public:
    // PhantomData equivalent: unused member just for type info
    [[maybe_unused]] static constexpr MH *_marker_mh = nullptr;
    // using base_class = IncomparableEncoding<MH>;
    using Parameter = typename MH::Parameter;
    using Randomness = typename MH::Randomness;

    const unsigned int DIMENSION = MH::DIMENSION;
    const unsigned int BASE = 1 << MH::BASE;
    const unsigned int MAX_TRIES = 100000;

    // Randomness rand() override
    // {
    //     return message_hash.rand();
    // }

    // Return Vector of unsigned 8-bit value: uint8_t
    tuple<vector<uint8_t>, int> encode(const Parameter &parameter, const std::array<uint8_t, MESSAGE_LENGTH> &message, const Randomness &randomness, uint32_t epoch)
    {
        // apply the message hash first to get chunks
        std::vector<uint8_t> chunks_message = MH::apply(parameter, epoch, randomness, message);
        uint32_t sum = 0;
        int valid = 0;

        // iterate over chunks
        for (uint8_t x : chunks_message)
        {
            uint32_t x_32 = static_cast<uint32_t>(x);
            sum += x;
        }

        // only output something if the chunk sum to the target sum
        if (static_cast<unsigned int>(sum) == TARGET_SUM)
        {
            valid = 0;
        }
        else
        {
            std::cout "Target Sum Mismatch!" << std::endl;
        }

        auto results = std::make_tuple(chunks_message, valid);
        return results;
    }

    static void internal_consistency_check()
    {
        assert !(BASE <= 1 << 8 && "Target Sum Encoding: Base must be at most 2^8");
        assert !(DIMENSION <= 1 << 8 && "Target Sum Encoding: Dimension must be at most 2^8");
        MH::internal_consistency_check();
    }
};
