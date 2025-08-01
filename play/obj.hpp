#include <iostream>
#include <vector>

class Obj {
    std::vector<int> data;
public:
    Obj() {
        std::cout << "Obj created" << std::endl;
    }

    ~Obj() {
        std::cout << "Obj destroyed" << std::endl;
    }

    virtual void doSomething();

    void addData(int value);

    void printData();
};

class Inc : public Obj {
public:
    Inc() {
        std::cout << "Inc created" << std::endl;
    }

    ~Inc() {
        std::cout << "Inc destroyed" << std::endl;
    }

    void doSomething() override {
        Obj::doSomething();
        std::cout << "Inc doing something additional." << std::endl;
    }
};