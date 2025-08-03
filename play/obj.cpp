#include <iostream>
#include <vector>

template<typename T>
void Obj<T>::doSomething() {
    std::cout << "Doing something with data of size: " << data.size() << std::endl;
}

template<typename T>
void Obj<T>::addData(T value) {
    data.push_back(value);
    std::cout << "Added value: " << value << " to data." << std::endl;
}

template<typename T>
void Obj<T>::printData() {
    std::cout << "Data contents: ";
    for (const auto& value : data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

template<typename T>
void Inc<T>::doSomething() {
    this->doSomething();
    std::cout << "Inc doing something additional." << std::endl;
}
