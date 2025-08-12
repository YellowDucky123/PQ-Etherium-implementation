# PQ-Etherium Proof of Concept 
This repository is a proof of concept for the PQ-Etherium Project with Hash-Based Multi-Signatures based on the paper 
@misc{cryptoeprint:2025/055,
      author = {Justin Drake and Dmitry Khovratovich and Mikhail Kudinov and Benedikt Wagner},
      title = {Hash-Based Multi-Signatures for Post-Quantum Ethereum},
      howpublished = {Cryptology {ePrint} Archive, Paper 2025/055},
      year = {2025},
      doi = {10.62056/aey7qjp10},
      url = {https://eprint.iacr.org/2025/055}
} 

The Signature Scheme itself is "Finished" (not tested), However, the PQ-SNARK aggregation is only "semi-finished".
Some OOP design paradigms might need to be changed for the Multi-Signature aggregation to Work, for reference, look at `generalized_xmss.hpp` and notice that to have a group of `vector<PublicKey>`, it proves to be quite hard with the current design as the MessageHash template argument has integers which aren't a finite field. 

## Proposed changes
Use "true" OOP design paradigm and use object instantiations for most of it. Limit type Static functions. This was done for half of the project (e.g. `ShaTweakHash` is under this design). 
However, things like the basic_winternitz was promptly changed to the more static approach, however, changed prove to be quite easy. 

### Effects
If this proposed changes were to be done, the multi signature aggregation algorithm might prove to be relatively easy. 
1. Move all number template arguments as class data.
2. ride the wave of effects from (1), e.g. make static functions non-static and use object instantiations.

However, it is also possible that by doing this, it might slow down the Scheme as more memory allocation and retrieval is required for the objects. 

# What is in the repository 
1. A Generalized XMSS implementation (untested)
2. A Basic Winternitz OTS Scheme
3. A Target-Sum Winternitz OTS Scheme
4. SHA messageHash, TweakHash, and PRF
5. Blake3 messageHash and TweakHash
6. An probable implementation of a multiSignature dataStructure/class
7. A **semi-Finished** implementation of a R1CS constraint system for aggregating the multi signatures with Aurora PQ-SNARK.
8. A "Finished" Python version of the Signature Scheme. ( The XMSS is **allegedly** Finished and tested in the Python version, Testing was not official )

# Building
Preffered version = C++ V23, -std=c++23
No concrete Build instructions have been made, However, here are the information needed to Build. 
1. Most Implementations (almost all) are in the headers, Therefore, to use, simply put the highest ranking header to use in a .cpp or .tcc, etc.
For example, to use the Winternitz OTS scheme, simply put ```#include "src/inc_encoding/basic_winternitz.hpp"``` in a .cpp or equivalent file.

2. When trying to compile the R1CS aggregate (if implementation is done) first compile the ```libiop``` submodule (look at their repository) and link it with the R1CS file.

# Files
File names are self explanatory

## Incomparable Encoding
- `src/inc_encoding/basic_winternitz.hpp`: A Basic Winternitz OTS
- `src/inc_encoding/target_sum.hpp`: A Target-Sum Winternitz OTS
- `src/symmetric/inc_encoding.hpp`: an abstract class for Incomparable Encoding

## XMSS
- `src/signature/generalized_xmss.hpp`: A General XMSS
- `src/symmetric/tweak_hash_tree.hpp`: Functions supporting the General XMSS

## SNARK
- `src/SNARK/myR1CS.hpp`: An R1CS for supposed to be for aggregating the Multi-Signatures
- `src/SNARK/aggregate.cpp`: A supposed aggregation use and instantiation

## Hashes
- `src/symmetric/message_hash.hpp`: An abstract class for message_hashes
- `src/symmetric/message_hash/sha.hpp`: A SHA256 instantiation for the message_hash
- `src/symmetric/message_hash/blake3.hpp`: A Blake3 instantiation for the message_hash
- `src/symmetric/TweakHash.hpp`: An abstract class for a Tweakable Hash function
- `src/symmetric/tweak_hash/sha.hpp`: A SHA256 Instantiation for a Tweakable Hash function
- `src/symmetric/tweak_hash/blake.hpp`: A Blake3 Instantiation for a Tweakable Hash function

## PRF
- `src/symmetric/prf.hpp`: An abstract class for pseudorandom functions
- `src/symmetric/prf/sha.hpp`: A SHA256 Pseudorandom Function instantiation

# Post Quantum Claim 
## Hash
On a not so analytical manner, This scheme is PQ because it uses Post Quantum secure hash Functions as it's underlying functionality. The SHA256 is one that has had many cryptoanalysis attacks done to, however, there are still no known attacks to SHA256 that are better than the Grover's attack at the time of this writing. The BLAKE3 is a newer hash function but also does not have known attacks better than Grover's attack.

## XMSS Many Time signature
The XMSS security claim is such that, it's underlying functionality is still an OTS. Furthermore, to limit it's data surface, this instantiation uses epochs and lifetimes which increase it's efficiency and security with "not so many" signatures under one public key.

## SNARK
Aurora's PQ-SNARK claim
