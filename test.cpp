#include <vector>
#include <variant>
#include <string>
#include <iostream>

template <typename T>
class MyClass {
private:
    T data;
public:
    MyClass(T val) : data(val) {}
    void print_data() const {
        std::cout << "Data: " << data << std::endl;
    }
};

using MyVariant = std::variant<MyClass<int>, MyClass<double>, MyClass<std::string>>;

int main() {
    std::vector<MyVariant> vec;
    vec.emplace_back(MyClass<int>(20));
    vec.emplace_back(MyClass<double>(2.718));
    vec.emplace_back(MyClass<std::string>("World"));

    auto obj = vec[0];
    obj.print_data();
}