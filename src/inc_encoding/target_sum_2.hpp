#pragma once

#include <vector>
#include <cstdint>
#include <array>
#include <random>
#include <concepts>
#include <type_traits>

#include "../inc_encoding_2.hpp"
#include "../symmetric/message_hash2.hpp"
#include "../serializable.hpp"
#include "lib.h"

// Clonable trait
template <typename T>
concept Cloneable = std::copy_constructible<T>;
