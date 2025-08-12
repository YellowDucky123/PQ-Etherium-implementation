#include <concepts>
#include "../symmetric/prf.hpp"
#include "../inc_encoding.hpp"
#include "../symmetric/TweakHash.hpp"
#include "../symmetric/tweak_hash_tree.hpp"
#include <cstdint>
#include <optional>

template <typename T>
concept PseudoRandom_c = requires(T t) {
    []<typename X, typename Y>(PseudoRandom<X, Y>&){}(t);
};

template <typename T>
concept IncomparableEncoding_c = requires(T t) {
    []<typename X, typename Y, uint A, uint B, uint C>(IncomparableEncoding<X, Y, A, B, C>&){}(t);
};

template <typename T>
concept TweakableHash_c = requires(T t) {
    []<typename X, typename Y, typename Z>(TweakableHash<X, Y, Z>&){}(t);
};

template <TweakableHash_c TH>
struct GeneralizedXMSSPublicKey {
    const typename TH::Domain root;
    const typename TH::Parameter parameter;

    PublicKey(typename TH::Domain _root_, typename TH::Parameter _parameter_) : root(_root_), parameter(_parameter_) {} 
};

template <PseudoRandom_c PRF, TweakableHash_c TH>
struct GeneralizedXMSSSecretKey {
    const typename PRF::Key prf_key;
    const HashTree<TH> tree;
    const typename TH::Parameter parameter;
    const uint activation_epoch;
    const uint num_active_epochs;
    
    SecretKey(const typename PRF::Key _prf_key_, const HashTree<TH> _tree_, const typename TH::Parameter _parameter_,
                const uint _activation_epoch_, const uint _num_active_epochs_) :
    prf_key(_prf_key_), tree(_tree_), parameter(_parameter_), activation_epoch(_activation_epoch_), 
    num_active_epochs(_num_active_epochs_) {}
};

template <IncomparableEncoding_c IE, TweakableHash_c TH>
struct GeneralizedXMSSSignature {
    const HashTreeOpening<TH> path;
    const typename IE::Randomness rho;
    const std::vector<typename TH::Domain> hashes;

    Signature(const HashTreeOpening<TH> _path_, const typename IE::Randomness _rho_,
        const std::vector<typename TH::Domain> _hashes_) :
    path(_path_), rho(_rho_), hashes(_hashes_) {}
};

template <IncomparableEncoding_c IE, TweakableHash_c TH, const uint attempts_t>
struct GeneralizedXMSSErrorNoSignature : public GeneralizedXMSSSignature<IE, TH> {
    static constexpr uint attempts = attempts_t;
};

template <PseudoRandom_c PRF, IncomparableEncoding_c IE, TweakableHash_c TH>
struct GeneralizedXMSSSignatureScheme {
    PRF prf;
    IE ie;
    TH th;

    GeneralizedXMSSSignatureScheme(PRF _prf_, IE _ie_, TH _th_) : prf(_prf_), ie(_ie_), th(_th_) {}
};

template <PseudoRandom_c PRF, IncomparableEncoding_c IE, TweakableHash_c TH, const uint LOG_LIFETIME>
struct SignatureScheme : public GeneralizedXMSSSignatureScheme<PRF, IE, TH>{
    using PublicKey = GeneralizedXMSSPublicKey<TH>;
    using SecretKey = GeneralizedXMSSSecretKey<PRF,TH>;
    using Signature = GeneralizedXMSSSignature<IE, TH>;

    using TH_domain = typename TH::Domain;

    SignatureScheme(TH _th_, PRF _prf_, IE, _ie_) : GeneralizedXMSSSignatureScheme(_th_, _prf_, _ie_) {};

    uint64_t LIFETIME = 1 << LOG_LIFETIME;

    std::tuple<PublicKey, SecretKey> key_gen(const uint activation_epoch, const uint num_active_epochs) {
        assert(
            activation_epoch + num_active_epochs <= static_cast<uint>(LIFETIME) &&
            "Key gen: `activation_epoch` and `num_active_epochs` are invalid for this lifetime"
        );

        auto parameter = th.rand_parameter();
        
        auto prf_key = prf.key_gen();

        auto num_chains = IE::DIMENSION;
        auto chain_length = IE::BASE;

        
        std::vector<TH_domain> chain_ends_hashes(num_active_epochs);

        #pragma omp parallel for
        for(uint epoch = activation_epoch; epoch < activation_epoch + num_active_epochs; epoch++) {
            std::vector<TH_domain> chain_ends(num_chains);
            #pragma omp parallel for 
            for(auto chain_index = 0; chain_index < num_chains; chain_index++) {
                TH::Domain start = static_cast<TH::Domain>(prf.apply(&prf_key, 
                    static_cast<uint32_t>(epoch), static_cast<uint64_t>(chain_index)));

                TH_domain out = chain<TH>(th, parameter, static_cast<uint32_t>(epoch), 
                                            static_cast<uint8_t>(chain_index), 0, chain_length - 1, start);
                chain_ends[chain_index] = out;
            }
            TH_domain outApply = th.apply(parameter, th.tree_tweak(0, static_cast<uint32_t>(epoch), chain_ends));
            chain_ends_hashes[epoch - activation_epoch] = outApply;
        }

        HashTree<TH> tree = HashTree<TH>::NewHashTree(LOG_LIFETIME, activation_epoch, parameter, chain_ends_hashes); 
        TH_domain root = tree.root();
        
        PublicKey pk = GeneralizedXMSSPublicKey(root, param_type);
        SecretKey sk = GeneralizedXMSSSecretKey(prf_key, tree, parameter, activation_epoch, num_active_epochs);

        return std::tuple<PublicKey, SecretKey>(pk, sk);
    }

    Signature sign(SecretKey sk, uint32_t epoch, std::vector<uint8_t> &message) {
        std::vector<uint> activation_range(sk.num_active_epochs);
        std::iota(activation_range.begin(), activation_range.end(), sk.activation_epoch);

        assert(
            std::find(activation_range.begin(), activation_range.end(), static_cast<uint>(epoch)) &&
            "Signing: key not active during this epoch"
        );

        using IE_randomness = typename IE::Randomness;

        HashTreeOpening<TH> path = sk.tree.path(epoch);
        uint max_tries = IE::MAX_TRIES;
        uint attempts = 0;
        std::optional<std::vector<uint8_t>> x_opt;
        std::optional<IE_randomness> rho_opt;

        std::vector<TH_domain> hashes(max_tries);
        while (attempts < max_tries) {
            IE_randomness curr_rho = IE::rand();
            std::vector<uint8_t> curr_x = IE::encode(static_cast<typename IE::param>(sk.parameter), epoch, static_cast<uint8_t>(chain_index), 0, steps, start);
            if (!curr_x.empty()) {
                rho_opt = curr_rho;
                x_opt = curr_x;
                break;
            }

            attempts++;
        }

        if(!x_opt.has_value()) {
            return GeneralizedXMSSErrorNoSignature<IE, TH, max_tries>();
        }
        
        assert(x_opt.has_value());
        std::vector<uint8_t> x = x_opt.value();
        assert(rho_opt.has_value());
        IE_randomness rho = rho_opt.value();

        uint num_chains = IE::DIMENSION;
        assert(
            x.size() == num_chains &&
            "Encoding is broken: returned too many or too few chunks."
        );

        std::vector<TH_domain> hashes(num_chains);
        #pragma omp parralel for
        for(uint chain_index = 0; chain_index < num_chains; chain_index++) {
            TH_domain start = static_cast<TH_domain>(prf.apply(sk.prf_key, epoch, static_cast<uint64_t>(chain_index)));
            uint steps = static_cast<uint>(x[chain_index]);
            TH_domain out = chain<TH>(sk.parameter, epoch, static_cast<uint8_t>(chain_index), 0, steps, start);
            hashes[chain_index] = out;
        }

        return GeneralizedXMSSSignature<IE, TH>(path, rho, hashes);
    }

    std::bool verify(PublicKey &pk, uint32_t epoch, std::vector<uint8_t> &message, Signature &sig) {
        assert(
            static_cast<uint64_t>(epoch) < LIFETIME &&
            "Generalized XMSS - Verify: Epoch too large."
        );

        using IE_parameter = typename IE::param;
        std::optional<std::vector<uint8_t>> x = IE::encode(static_cast<IE_parameter>(pk.parameter), message, sig.rho, epoch);

        if(x.empty()) {
            return false;
        }

        assert(x.has_value());
        x = x.value();

        uint chain_length = IE::BASE;
        uint num_chains = IE::DIMENSION;

        assert(
            x.size() == num_chains &&
            "Encoding is broken: returned too many or too few chunks."
        );

        std::vector<TH_domain> chain_ends(num_chains);

        for(int chain_index = 0; chain_index < x.size(); chain_index++) {
            uint8_t xi = x[chain_index];

            uint8_t steps = static_cast<uint8_t>(chain_length - 1) - xi;
            uint8_t start_pos_in_chain = xi;
            TH_domain &start = sig.hashes[chain_index];
            TH_domain end = chain<TH>(pk.parameter, epoch, static_cast<uint8_t>(chain_index), start_pos_in_chain, static_cast<uint>(steps), start);
            chain_ends[chain_index] = end;
        }

        return hash_tree_verify(
            pk.parameter,
            pk.root,
            epoch,
            chain_ends,
            sig.path
        );
    }
};