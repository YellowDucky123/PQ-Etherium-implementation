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

    void doSomething();

    void addData(int value);

    void printData();
};