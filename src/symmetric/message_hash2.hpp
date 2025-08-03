#pragma once

#include <vector>
#include <cstdint>
#include <array>
#include <random>
#include <concepts>
#include <type_traits>
#include "../serializable.hpp"
#include "../lib.h"

#include <concepts>

template <typename T>
concept Cloneable = std::copy_constructible<T>;

// Message Hash Trait
template <typename MH>
concept MessageHash = requires {
        typename MH::Parameter;
        typename MH::Randomness;

        { MH::DIMENSION } -> std::convertible_to<std::size_t>;
        { MH::BASE } -> std::convertible_to<std::size_t>;

        // methods
        // generate randomness
        { MH::rand() } -> std::same_as<typename MH::Randomness>;
        /// Applies the message hash to a parameter, an epoch,
        /// a randomness, and a message. It outputs a list of chunks.
        /// The list contains DIMENSION many elements, each between
        /// 0 and BASE - 1 (inclusive).
        { MH::apply(
            // parameter
            std::declval<typename MH::Parameter>(),
            // randomness
            std::declval<typename MH::Randomness>(),
            // message
            std::declval<std::array<uint8_t, MESSAGE_LENGTH>>(),
            // epoch
            std::declval<uint32_t>()) } -> std::same_as<std::vector<uint8_t>>;
} && Serializable<typename MH::Parameter> && Serializable<typename MH::Randomness> && std::is_copy_constructible_v<typename MH::Parameter> && std::is_default_constructible_v<typename MH::Parameter>;