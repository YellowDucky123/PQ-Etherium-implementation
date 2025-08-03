#include <concepts>
#include <vector>
#include <random>
#include "lib.h"
#include "serializable.hpp"

template <typename T>
concept IncomparableEncoding = Serializable<typename T::Parameter> && Serializable<typename T::Randomness> && requires {
        typename T::Parameter;
        typename T::Randomness;
        { T::DIMENSION } -> std::convertible_to<std::size_t>;
        { T::MAX_TRIES } -> std::convertible_to<std::size_t>;
        { T::BASE } -> std::convertible_to<std::size_t>;
        { T{}.encode(std::declval<typename T::Parameter>(), std::declval<std::array<uint8_t, MESSAGE_LENGTH>>(), std::declval<typename T::Randomness>(), std::declval<uint32_t>()) } -> std::same_as<std::vector<uint8_t>>;
};
