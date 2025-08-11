#include <cstdint>
#include <tuple>

template <typename PublicKey, typename SecretKey, typename Signature>
struct SignatureScheme
{
    const uint64_t LIFETIME;

    virtual std::tuple<PublicKey, SecretKey> key_gen(uint activation_epoch, uint num_active_epochs) = 0;

    virtual Signature sign(const SecretKey &sk, uint32_t epoch, const std::vector<uint8_t> msg) = 0;

    virtual bool verify(const PublicKey &pk, uint32_t epoch, const std::vector<uint8_t> msg, const Signature &sig) = 0;

    virtual void internal_consistency_check() = 0;
};