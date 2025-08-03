#include <iostream>
#include <vector>

template<typename T>
class Obj {
    std::vector<T> data;
public:
    Obj() {
        std::cout << "Obj created" << std::endl;
    }

    ~Obj() {
        std::cout << "Obj destroyed" << std::endl;
    }

    virtual void doSomething();

    void addData(T value);

    void printData();
};

template<typename T>
class Inc : public Obj<T> {
public:
    Inc() {
        std::cout << "Inc created" << std::endl;
    }

    ~Inc() {
        std::cout << "Inc destroyed" << std::endl;
    }

    void doSomething() override; 
};

#include "obj.cpp"

