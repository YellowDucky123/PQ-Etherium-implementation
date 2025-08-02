#include <cstdint>

template <typename Key, typename Output>
class PseudoRandom {
public:
    virtual Key key_gen() = 0;

    virtual Output apply(Key key, uint32_t epoch, uint64_t index) = 0;

    virtual void internal_consistency_check() = 0;
};