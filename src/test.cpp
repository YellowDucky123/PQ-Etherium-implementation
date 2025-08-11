#include <iostream>

template <int x>
struct test {
    static constexpr int X = x;
};

template <typename T>
struct t {
    static void print() {
        std::cout << T::X << '\n';
    }
};

int main() {
    // test<1> testObj;
    t<test<1>>::print();
}