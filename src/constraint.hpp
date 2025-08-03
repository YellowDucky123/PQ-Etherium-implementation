#pragma once

#include "symmetric/message_hash.hpp"
#include <concepts>
#include <any>

template <typename T>
concept MessageHash_c = requires(T t) {
    []<typename X, typename Y>(MessageHash<X, Y>&){}(t);
};

