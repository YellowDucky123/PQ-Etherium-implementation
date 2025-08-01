#include "obj.hpp"
#include <concepts>


template<typename O, typename T>
requires std::is_base_of_v<Obj, O> && std::is_base_of_v<Obj, T>
class Som {
    O s = O();
public:
    void print() {
        s.doSomething();
    }
};