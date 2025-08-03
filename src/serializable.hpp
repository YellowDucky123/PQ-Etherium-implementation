#pragma once

#include <string>
#include <concepts>
#include <type_traits>

// Defines a Serializable concept: template parameters
template <typename T>
concept Serializable = requires(T a, std::string s) {
        // Takes an object of type T and returns a std::string
        { serialize(a) } -> std::same_as<std::string>;
        // Takes a std:: string and returns an object of type T.
        { deserialize<T>(s) } -> std::same_as<T>;
};
