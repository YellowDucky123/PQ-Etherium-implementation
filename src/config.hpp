#include <cstdint>

#ifndef CONFIG_HPP
#define CONFIG_HPP

inline constexpr uint8_t MESSAGE_LENGTH = 32;
inline constexpr uint8_t TWEAK_SEPARATOR_FOR_MESSAGE_HASH = 0x02;
inline constexpr uint8_t TWEAK_SEPARATOR_FOR_TREE_HASH = 0x01;
inline constexpr uint8_t TWEAK_SEPARATOR_FOR_CHAIN_HASH = 0x00;

#endif
