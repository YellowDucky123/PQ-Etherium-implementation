#include <iostream>
#include <unordered_map>
#include "../catch_amalgamated.hpp"
#include "../../src/symmetric/tweak_hash/sha.hpp"
#include "../../src/symmetric/TweakHashChain.hpp"
#include "../../random2.hpp"

TEST_CASE("test_apply: ShaTweak128128 tree tweak")
{
      ShaTweakHash ShaTweak128128 = ShaTweakHash(16, 16);
      ShaTweak128128.internal_consistency_check();

      auto parameter = ShaTweak128128.rand_parameter();
      auto message_1 = ShaTweak128128.rand_domain();
      auto message_2 = ShaTweak128128.rand_domain();
      auto tree_tweak = ShaTweak128128.tree_tweak(0, 3);

      std::vector<std::decay_t<decltype(message_1)>::value_type> messages;
      messages.reserve(message_1.size() + message_2.size());
      messages.insert(messages.end(), message_1.begin(), message_1.end());
      messages.insert(messages.end(), message_2.begin(), message_2.end());

      ShaTweak128128.apply(parameter, *tree_tweak, messages);
}

TEST_CASE("test_apply: ShaTweak128128 chain tweak")
{
      ShaTweakHash ShaTweak128128 = ShaTweakHash(16, 16);
      ShaTweak128128.internal_consistency_check();

      auto parameter = ShaTweak128128.rand_parameter();
      auto message_1 = ShaTweak128128.rand_domain();
      auto message_2 = ShaTweak128128.rand_domain();
      auto chain_tweak = ShaTweak128128.chain_tweak(2, 3, 4);

      std::vector<std::decay_t<decltype(message_1)>::value_type> messages;
      messages.reserve(message_1.size() + message_2.size());
      messages.insert(messages.end(), message_1.begin(), message_1.end());
      messages.insert(messages.end(), message_2.begin(), message_2.end());

      ShaTweak128128.apply(parameter, *chain_tweak, messages);
}

TEST_CASE("test_apply: ShaTweak128192 tree tweak")
{
      ShaTweakHash ShaTweak128192 = ShaTweakHash(16, 24);
      ShaTweak128192.internal_consistency_check();

      auto parameter = ShaTweak128192.rand_parameter();
      auto message_1 = ShaTweak128192.rand_domain();
      auto message_2 = ShaTweak128192.rand_domain();
      auto tree_tweak = ShaTweak128192.tree_tweak(0, 3);

      std::vector<std::decay_t<decltype(message_1)>::value_type> messages;
      messages.reserve(message_1.size() + message_2.size());
      messages.insert(messages.end(), message_1.begin(), message_1.end());
      messages.insert(messages.end(), message_2.begin(), message_2.end());

      ShaTweak128192.apply(parameter, *tree_tweak, messages);
}

TEST_CASE("test_apply: ShaTweak128192 chain tweak")
{
      ShaTweakHash ShaTweak128192 = ShaTweakHash(16, 24);
      ShaTweak128192.internal_consistency_check();

      auto parameter = ShaTweak128192.rand_parameter();
      auto message_1 = ShaTweak128192.rand_domain();
      auto message_2 = ShaTweak128192.rand_domain();
      auto chain_tweak = ShaTweak128192.chain_tweak(2, 3, 4);

      std::vector<std::decay_t<decltype(message_1)>::value_type> messages;
      messages.reserve(message_1.size() + message_2.size());
      messages.insert(messages.end(), message_1.begin(), message_1.end());
      messages.insert(messages.end(), message_2.begin(), message_2.end());

      std::vector<uint8_t> result = ShaTweak128192.apply(parameter, *chain_tweak, messages);
}

TEST_CASE("test_apply: ShaTweak192192 tree tweak")
{
      ShaTweakHash ShaTweak192192 = ShaTweakHash(24, 24);
      ShaTweak192192.internal_consistency_check();

      auto parameter = ShaTweak192192.rand_parameter();
      auto message_1 = ShaTweak192192.rand_domain();
      auto message_2 = ShaTweak192192.rand_domain();
      auto tree_tweak = ShaTweak192192.tree_tweak(0, 3);

      std::vector<std::decay_t<decltype(message_1)>::value_type> messages;
      messages.reserve(message_1.size() + message_2.size());
      messages.insert(messages.end(), message_1.begin(), message_1.end());
      messages.insert(messages.end(), message_2.begin(), message_2.end());

      ShaTweak192192.apply(parameter, *tree_tweak, messages);
}

TEST_CASE("test_apply: ShaTweak192192 chain tweak")
{
      ShaTweakHash ShaTweak192192 = ShaTweakHash(24, 24);
      ShaTweak192192.internal_consistency_check();

      auto parameter = ShaTweak192192.rand_parameter();
      auto message_1 = ShaTweak192192.rand_domain();
      auto message_2 = ShaTweak192192.rand_domain();
      auto chain_tweak = ShaTweak192192.chain_tweak(2, 3, 4);

      std::vector<std::decay_t<decltype(message_1)>::value_type> messages;
      messages.reserve(message_1.size() + message_2.size());
      messages.insert(messages.end(), message_1.begin(), message_1.end());
      messages.insert(messages.end(), message_2.begin(), message_2.end());

      ShaTweak192192.apply(parameter, *chain_tweak, messages);
}

TEST_CASE("Test tree tweak injective")
{
      ShaTweakHash ShaTweak192192 = ShaTweakHash(24, 24);

      for (int i = 0; i < 100000; i++)
      {
            uint32_t epoch = Random::generate<uint32_t>();
            uint8_t chain_index = Random::generate<uint8_t>();
            uint8_t pos_in_chain = Random::generate<uint8_t>();

            // ShaTreeTweak tweak_encoding = ShaTreeTweak
      }
}