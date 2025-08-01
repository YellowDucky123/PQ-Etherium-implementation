#include "obj.hpp"

#include <iostream>
#include <vector>

void Obj::doSomething() {
    std::cout << "Doing something with data of size: " << data.size() << std::endl;
}

void Obj::addData(int value) {
    data.push_back(value);
    std::cout << "Added value: " << value << " to data." << std::endl;
}

void Obj::printData() {
    std::cout << "Data contents: ";
    for (const auto& value : data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}
