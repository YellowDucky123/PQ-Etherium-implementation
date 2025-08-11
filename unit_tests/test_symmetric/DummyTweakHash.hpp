#include <array>
#include <vector>
#include <cstdint>
#include <iostream>
#include "../../src/symmetric/TweakHash.hpp"
#include "../../src/random.hpp"
#include "../../src/endian.hpp"

struct DummyParameter
{
      uint64_t value;

      DummyParameter() : value(0) {}
      DummyParameter(uint64_t v) : value(v) {}
};

struct DummyTweak
{
      std::array<uint8_t, 32> data;

      DummyTweak()
      {
            data.fill(0);
      }

      DummyTweak(const std::array<uint8_t, 32> &d) : data(d) {}
};

struct DummyDomain
{
      uint8_t data[32];
      DummyDomain()
      {
            memset(data, 0, sizeof(data));
      }

      DummyDomain(uint8_t fill_value)
      {
            memset(data, fill_value, sizeof(data));
      }

      bool operator==(const DummyDomain &other) const
      {
            return memcmp(data, other.data, sizeof(data)) == 0;
      }
};

// Dummy implementation of TweakableHash
class DummyTweakableHash : public TweakableHash<DummyParameter, DummyTweak, DummyDomain>
{
      Parameter rand_parameter() override
      {
            CryptoRng<DummyParameter> rng;
            Parameter parameter = rng.generate();
            return DummyParameter(parameter);
      }

      Domain rand_domain() override
      {
            CryptoRng<DummyDomain> rng;
            DummyDomain domain = rng.generate();
            return DummyDomain(domain);
      }

      /// Returns a tweak to be used in the Merkle tree.
      /// Note: this is assumed to be distinct from the outputs of chain_tweak
      // How is this applied, need to use to test for BLAKE3 / SHA3
      Tweak tree_tweak(uint8_t level, uint32_t pos_in_level) override
      {
            endian e;
            std::array<uint8_t, 32> tweak_data;
            tweak_data.fill(0);

            tweak_data[0] = 1;
            tweak_data[1] = level;

            std::vector<uint8_t> be_pos_in_level = e.to_be_bytes(pos_in_level);

            int n = 2;
            for (int i = 0; i < be_pos_in_level.size(); i++)
            {
                  tweak_data[n] = be_pos_in_level[0];
                  n += 1;
            }

            return DummyTweak(tweak_data);
      }

      /// Returns a tweak to be used in chains.
      /// Note: this is assumed to be distinct from the outputs of tree_tweak
      Tweak chain_tweak(uint32_t epoch, uint8_t chain_index, uint8_t pos_in_chain) override
      {
            endian e;

            std::array<uint8_t, 32> tweak_data;
            tweak_data.fill(0);

            tweak_data[0] = 0;

            // Write epoch in big-endian
            std::vector<uint8_t> be_epoch = e.to_be_bytes(epoch);

            // Insert into tweak_data
            int n = 1;
            for (int i = 0; i < be_epoch.size(); i++)
            {
                  tweak_data[n] = be_epoch[i];
                  n += 1;
            }

            tweak_data[n] = chain_index;
            tweak_data[n + 1] = pos_in_chain;

            return DummyTweak(tweak_data);
      }

      Domain apply(Parameter parameter, Tweak &tweak, std::vector<Domain> &message)
      {
            // Need to find out what this is!!
      }
};
