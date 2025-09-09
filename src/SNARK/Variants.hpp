#pragma once

#include <variant>
#include "symmetric/tweak_hash/sha.hpp"
#include "symmetric/tweak_hash/blake.hpp"
#include "symmetric/message_hash.hpp"

using pk_variants = variant<
    GeneralizedXMSSPublicKey<ShaTweakHash>, 
    GeneralizedXMSSPublicKey<BlakeTweakHash>
>;

using sig_variants = variant<
    GeneralizedXMSSSignature<MessageHash<std::vector<uint8_t>, std::vector<uint8_t>>, ShaTweakHash>,
    GeneralizedXMSSSignature<MessageHash<std::vector<uint8_t>, std::vector<uint8_t>>, BlakeTweakHash>    
>;
