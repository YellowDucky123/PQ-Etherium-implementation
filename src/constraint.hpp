#include "symmetric/message_hash.hpp"
#include <concepts>

template <typename T>
concept MessageHash_c = std::is_base_of_v<MessageHash, T>;