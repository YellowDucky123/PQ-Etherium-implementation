#include <concepts>
#include <vector>
#include <random>
#include <openssl/rand.h>
#include "lib.h"
#include "serializable.hpp"

template <typename E>
concept IncomparableEncoding = requires {
        // associated types
        typename E::Parameter;
        typename E::Randomness;

        // associated constants
        { E::DIMENSION } -> std::convertible_to<std::size_t>;
        { E::MAX_TRIES } -> std::convertible_to<std::size_t>;
        { E::BASE } -> std::convertible_to<std::size_t>;

        // methods
        { E::rand() } -> std::same_as<typename E::Randomness>;
        { E::encode(
            // parameter
            std::declval<typename E::Parameter>(),
            // message
            std::declval<std::array<uint8_t, MESSAGE_LENGTH>>(),
            // randomness
            std::declval<typename E::Randomness>(),
            // epoch
            std::declval<uint32_t>()) } -> std::same_as<std::vector<uint8_t>>;
} && Serializable<typename E::Parameter> && Serializable<typename E::Randomness>;
