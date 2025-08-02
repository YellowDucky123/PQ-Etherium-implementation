#include "obj.hpp"

int main() {
    Obj<int> obj;
    obj.addData(10);
    obj.addData(20);
    obj.printData();
    
    return 0;
}