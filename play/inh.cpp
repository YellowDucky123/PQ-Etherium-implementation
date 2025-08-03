#include <concepts>
#include <iostream>

template<typename T>
class A {
public:
    typedef T value_type;
    const static int x = 10;
};

template<class Value> 
class Holder {
public:
    typedef Value value_type;

    int i;

    Holder() {
        this->i = 1;
    } 
    value_type m_val; // member variable
};

template< class THolder >
void SomeMethod( THolder const& holder, int i) {
    using x = typename THolder::value_type; // using alias for value_type
    x v= i;
    std::cout << v;
}

class Holder2 : public Holder<int> {
public: 
    
    using base = Holder<int>;
    Holder2() : base() {
        this->i = 2; // initialize base class member
    }
    void print() {
        std::cout << base::i;
    }
};

int main() {
    Holder<int> a;
    Holder2 b;

    b.print();
}