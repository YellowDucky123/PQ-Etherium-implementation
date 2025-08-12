#include <iostream>
#include <unordered_map>
#include <optional>
#include <vector>
#include <tuple>
#include "../catch_amalgamated.hpp"
#include "../../src/symmetric/tweak_hash/sha.hpp"
#include "../../src/symmetric/TweakHash.hpp"
#include "../../src/random2.hpp"
struct VectorHasher
{
      std::size_t operator()(const std::vector<uint8_t> &v) const
      {
            std::size_t hash = 0;
            for (auto b : v)
            {
                  hash = (hash * 31) ^ b; // simple hash combine
            }
            return hash;
      }
};

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

      for (int i = 0; i < 1000; i++)
      {
            uint8_t level1 = Random::generate<uint8_t>();
            uint32_t pos_in_level1 = Random::generate<uint32_t>();

            std::unique_ptr<ShaTweak> tweak1 = std::make_unique<ShaTreeTweak>(level1, pos_in_level1);
            std::vector<uint8_t> tweak_encoding1 = tweak1->to_bytes();

            std::unordered_map<std::vector<uint8_t>, std::pair<uint8_t, uint32_t>, VectorHasher> map;
            auto it1 = map.find(tweak_encoding1);
            if (it1 != map.end())
            {
                  // Key exists — check for collision
                  auto &[prev_level1, prev_pos1] = it1->second;

                  // Assert match, else print error and abort
                  REQUIRE(!(prev_level1 == level1 && prev_pos1 == pos_in_level1));
                  if (!(prev_level1 == level1 && prev_pos1 == pos_in_level1))
                  {
                        std::cerr << "Collision detected for (" << +prev_level1 << ", " << prev_pos1
                                  << ") and (" << +level1 << ", " << pos_in_level1
                                  << ") with output [";

                        for (auto b : tweak_encoding1)
                              std::cerr << std::hex << +b << " ";
                        std::cerr << "]\n";

                        assert(false && "Collision detected in tweak_encoding map!");
                  }
                  else
                  {
                        // Insert new entry
                        map.emplace(tweak_encoding1, std::make_pair(level1, pos_in_level1));
                  }
            }

            // inputs with common level
            std::unordered_map<std::vector<uint8_t>, std::pair<uint8_t, uint32_t>, VectorHasher> map2;
            uint8_t level2 = Random::generate<uint8_t>();

            for (int i = 0; i < 1000; i++)
            {
                  uint32_t pos_in_level2 = Random::generate<uint32_t>();
                  std::unique_ptr<ShaTweak> tweak2 = std::make_unique<ShaTreeTweak>(level2, pos_in_level2);
                  std::vector<uint8_t> tweak_encoding2 = tweak2->to_bytes();
                  auto [it2, inserted] = map2.emplace(tweak_encoding2, std::make_pair(level2, pos_in_level2));

                  if (!inserted)
                  {
                        auto &prev_value2 = it2->second;
                        uint8_t prev_level2 = prev_value2.first;
                        uint32_t prev_pos_in_level2 = prev_value2.second;

                        REQUIRE(prev_pos_in_level2 != pos_in_level2);
                        if (prev_pos_in_level2 != pos_in_level2)
                        {
                              std::cerr << "Collision detected2 for (" << +level2 << ", " << prev_pos_in_level2
                                        << ") and (" << +level2 << ", " << pos_in_level2
                                        << ") with output [";

                              for (auto b : tweak_encoding2)
                                    std::cerr << std::hex << +b << " ";
                              std::cerr << "]\n";

                              assert(false && "Collision detected in tweak_encoding map!");
                        }
                  }
            }

            // inputs with coommon pos_in_level
            std::unordered_map<std::vector<uint8_t>, uint8_t, VectorHasher> map3;
            uint32_t pos_in_level3 = Random::generate<uint32_t>();
            for (int i = 0; i < 1000; i++)
            {
                  uint8_t level3 = Random::generate<uint8_t>();
                  std::unique_ptr<ShaTweak> tweak3 = std::make_unique<ShaTreeTweak>(level3, pos_in_level3);
                  std::vector<uint8_t> tweak_encoding3 = tweak3->to_bytes();
                  auto [it3, inserted3] = map3.emplace(tweak_encoding3, level3);
                  if (!inserted3)
                  {
                        auto &prev_value3 = it3->second;
                        uint8_t prev_level3 = prev_value3;

                        REQUIRE(prev_level3 != level3);
                        if (prev_level3 != level3)
                        {
                              std::cerr << "Collision detected for (" << +level3 << ", " << pos_in_level3
                                        << ") with output [";

                              for (auto b : tweak_encoding3)
                                    std::cerr << std::hex << +b << " ";
                              std::cerr << "]\n";

                              assert(false && "Collision detected in tweak_encoding map!");
                        }
                  }
            }
      }
}

TEST_CASE("Test chain tweak hash: injective")
{
      std::unordered_map<std::vector<uint8_t>, std::tuple<uint32_t, uint8_t, uint8_t>, VectorHasher> map1;

      for (int i = 0; i < 1000; i++)
      {
            uint32_t epoch1 = Random::generate<uint32_t>();
            uint8_t chain_index1 = Random::generate<uint8_t>();
            uint8_t pos_in_chain1 = Random::generate<uint8_t>();

            std::tuple<uint32_t, uint8_t, uint8_t> input1(epoch1, chain_index1, pos_in_chain1);

            std::unique_ptr<ShaTweak> tweak1 = std::make_unique<ShaChainTweak>(epoch1, chain_index1, pos_in_chain1);
            std::vector<uint8_t> tweak_encoding1 = tweak1->to_bytes();

            auto [it1, inserted1] = map1.emplace(tweak_encoding1, input1);
            if (!inserted1)
            {
                  auto &prev_value1 = it1->second;
                  REQUIRE(prev_value1 == input1);
                  if (prev_value1 != input1)
                  {
                        std::cerr << "Collision detected for inputs with different values!\n";
                        assert(false && "Collision with different inputs detected!");
                  }
            }
      }

      // inputs with fixed chain_index
      std::unordered_map<std::vector<uint8_t>, std::tuple<uint32_t, uint8_t, uint8_t>, VectorHasher> map2;
      uint8_t chain_index2 = Random::generate<uint8_t>();
      for (int i = 0; i < 1000; i++)
      {
            uint32_t epoch2 = Random::generate<uint32_t>();
            uint8_t pos_in_chain2 = Random::generate<uint8_t>();

            std::tuple<uint32_t, uint8_t, uint8_t> input2(epoch2, chain_index2, pos_in_chain2);

            std::unique_ptr<ShaTweak> tweak2 = std::make_unique<ShaChainTweak>(epoch2, chain_index2, pos_in_chain2);
            std::vector<uint8_t> tweak_encoding2 = tweak2->to_bytes();

            auto [it2, inserted2] = map2.emplace(tweak_encoding2, input2);
            if (!inserted2)
            {
                  auto &prev_value2 = it2->second;
                  REQUIRE(prev_value2 == input2);
                  if (prev_value2 != input2)
                  {
                        std::cerr << "Collision detected for inputs with different values!\n";
                        assert(false && "Collision with different inputs detected!");
                  }
            }
      }

      // inputs with fixed chain_index
      std::unordered_map<std::vector<uint8_t>, std::tuple<uint32_t, uint8_t, uint8_t>, VectorHasher> map3;
      uint8_t pos_in_chain3 = Random::generate<uint8_t>();

      for (int i = 0; i < 1000; i++)
      {
            uint32_t epoch3 = Random::generate<uint32_t>();
            uint8_t chain_index3 = Random::generate<uint8_t>();

            std::tuple<uint32_t, uint8_t, uint8_t> input3(epoch3, chain_index3, pos_in_chain3);

            std::unique_ptr<ShaTweak> tweak3 = std::make_unique<ShaChainTweak>(epoch3, chain_index2, pos_in_chain3);
            std::vector<uint8_t> tweak_encoding3 = tweak3->to_bytes();

            auto [it3, inserted3] = map3.emplace(tweak_encoding3, input3);
            if (!inserted3)
            {
                  auto &prev_value3 = it3->second;
                  REQUIRE(prev_value3 == input3);
                  if (prev_value3 != input3)
                  {
                        std::cerr << "Collision detected for inputs with different values!\n";
                        assert(false && "Collision with different inputs detected!");
                  }
            }

            for (int i = 0; i < 1000; i++)
            {
                  uint8_t level1 = Random::generate<uint8_t>();
                  uint32_t pos_in_level1 = Random::generate<uint32_t>();

                  std::unique_ptr<ShaTweak> tweak1 = std::make_unique<ShaTreeTweak>(level1, pos_in_level1);
                  std::vector<uint8_t> tweak_encoding1 = tweak1->to_bytes();

                  std::unordered_map<std::vector<uint8_t>, std::pair<uint8_t, uint32_t>, VectorHasher> map;
                  auto it1 = map.find(tweak_encoding1);
                  if (it1 != map.end())
                  {
                        // Key exists — check for collision
                        auto &[prev_level1, prev_pos1] = it1->second;

                        // Assert match, else print error and abort
                        REQUIRE(!(prev_level1 == level1 && prev_pos1 == pos_in_level1));
                        if (!(prev_level1 == level1 && prev_pos1 == pos_in_level1))
                        {
                              std::cerr << "Collision detected for (" << +prev_level1 << ", " << prev_pos1
                                        << ") and (" << +level1 << ", " << pos_in_level1
                                        << ") with output [";

                              for (auto b : tweak_encoding1)
                                    std::cerr << std::hex << +b << " ";
                              std::cerr << "]\n";

                              assert(false && "Collision detected in tweak_encoding map!");
                        }
                        else
                        {
                              // Insert new entry
                              map.emplace(tweak_encoding1, std::make_pair(level1, pos_in_level1));
                        }
                  }

                  // inputs with common level
                  std::unordered_map<std::vector<uint8_t>, std::pair<uint8_t, uint32_t>, VectorHasher> map2;
                  uint8_t level2 = Random::generate<uint8_t>();

                  for (int i = 0; i < 1000; i++)
                  {
                        uint32_t pos_in_level2 = Random::generate<uint32_t>();
                        std::unique_ptr<ShaTweak> tweak2 = std::make_unique<ShaTreeTweak>(level2, pos_in_level2);
                        std::vector<uint8_t> tweak_encoding2 = tweak2->to_bytes();
                        auto [it2, inserted] = map2.emplace(tweak_encoding2, std::make_pair(level2, pos_in_level2));

                        if (!inserted)
                        {
                              auto &prev_value2 = it2->second;
                              uint8_t prev_level2 = prev_value2.first;
                              uint32_t prev_pos_in_level2 = prev_value2.second;

                              REQUIRE(prev_pos_in_level2 != pos_in_level2);
                              if (prev_pos_in_level2 != pos_in_level2)
                              {
                                    std::cerr << "Collision detected2 for (" << +level2 << ", " << prev_pos_in_level2
                                              << ") and (" << +level2 << ", " << pos_in_level2
                                              << ") with output [";

                                    for (auto b : tweak_encoding2)
                                          std::cerr << std::hex << +b << " ";
                                    std::cerr << "]\n";

                                    assert(false && "Collision detected in tweak_encoding map!");
                              }
                        }
                  }

                  // inputs with common pos_in_level
                  std::unordered_map<std::vector<uint8_t>, uint8_t, VectorHasher> map3;
                  uint32_t pos_in_level3 = Random::generate<uint32_t>();
                  for (int i = 0; i < 1000; i++)
                  {
                        uint8_t level3 = Random::generate<uint8_t>();
                        std::unique_ptr<ShaTweak> tweak3 = std::make_unique<ShaTreeTweak>(level3, pos_in_level3);
                        std::vector<uint8_t> tweak_encoding3 = tweak3->to_bytes();
                        auto [it3, inserted3] = map3.emplace(tweak_encoding3, level3);
                        if (!inserted3)
                        {
                              auto &prev_value3 = it3->second;
                              uint8_t prev_level3 = prev_value3;

                              REQUIRE(prev_level3 != level3);
                              if (prev_level3 != level3)
                              {
                                    std::cerr << "Collision detected for (" << +level3 << ", " << pos_in_level3
                                              << ") with output [";

                                    for (auto b : tweak_encoding3)
                                          std::cerr << std::hex << +b << " ";
                                    std::cerr << "]\n";

                                    assert(false && "Collision detected in tweak_encoding map!");
                              }
                        }
                  }
            }
      }
}

TEST_CASE("Test chain tweak hash: injective")
{

      std::unordered_map<std::vector<uint8_t>, std::tuple<uint32_t, uint8_t, uint8_t>, VectorHasher> map1;

      for (int i = 0; i < 1000; i++)
      {
            uint32_t epoch1 = Random::generate<uint32_t>();
            uint8_t chain_index1 = Random::generate<uint8_t>();
            uint8_t pos_in_chain1 = Random::generate<uint8_t>();

            std::tuple<uint32_t, uint8_t, uint8_t> input1(epoch1, chain_index1, pos_in_chain1);

            std::unique_ptr<ShaTweak> tweak1 = std::make_unique<ShaChainTweak>(epoch1, chain_index1, pos_in_chain1);
            std::vector<uint8_t> tweak_encoding1 = tweak1->to_bytes();

            auto [it1, inserted1] = map1.emplace(tweak_encoding1, input1);
            if (!inserted1)
            {
                  auto &prev_value1 = it1->second;
                  REQUIRE(prev_value1 == input1);
                  if (prev_value1 != input1)
                  {
                        std::cerr << "Collision detected for inputs with different values!\n";
                        assert(false && "Collision with different inputs detected!");
                  }
            }
      }

      // inputs with fixed chain_index
      std::unordered_map<std::vector<uint8_t>, std::tuple<uint32_t, uint8_t, uint8_t>, VectorHasher> map2;
      uint8_t chain_index2 = Random::generate<uint8_t>();
      for (int i = 0; i < 1000; i++)
      {
            uint32_t epoch2 = Random::generate<uint32_t>();
            uint8_t pos_in_chain2 = Random::generate<uint8_t>();

            std::tuple<uint32_t, uint8_t, uint8_t> input2(epoch2, chain_index2, pos_in_chain2);

            std::unique_ptr<ShaTweak> tweak2 = std::make_unique<ShaChainTweak>(epoch2, chain_index2, pos_in_chain2);
            std::vector<uint8_t> tweak_encoding2 = tweak2->to_bytes();

            auto [it2, inserted2] = map2.emplace(tweak_encoding2, input2);
            if (!inserted2)
            {
                  auto &prev_value2 = it2->second;
                  REQUIRE(prev_value2 == input2);
                  if (prev_value2 != input2)
                  {
                        std::cerr << "Collision detected for inputs with different values!\n";
                        assert(false && "Collision with different inputs detected!");
                  }
            }
      }

      // inputs with fixed chain_index
      std::unordered_map<std::vector<uint8_t>, std::tuple<uint32_t, uint8_t, uint8_t>, VectorHasher> map3;
      uint8_t pos_in_chain3 = Random::generate<uint8_t>();

      for (int i = 0; i < 1000; i++)
      {
            uint32_t epoch3 = Random::generate<uint32_t>();
            uint8_t chain_index3 = Random::generate<uint8_t>();

            std::tuple<uint32_t, uint8_t, uint8_t> input3(epoch3, chain_index3, pos_in_chain3);

            std::unique_ptr<ShaTweak> tweak3 = std::make_unique<ShaChainTweak>(epoch3, chain_index2, pos_in_chain3);
            std::vector<uint8_t> tweak_encoding3 = tweak3->to_bytes();

            auto [it3, inserted3] = map3.emplace(tweak_encoding3, input3);
            if (!inserted3)
            {
                  auto &prev_value3 = it3->second;
                  REQUIRE(prev_value3 == input3);
                  if (prev_value3 != input3)
                  {
                        std::cerr << "Collision detected for inputs with different values!\n";
                        assert(false && "Collision with different inputs detected!");
                  }
            }
      }
}