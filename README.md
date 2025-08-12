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

The Signature Scheme itself is "Finished" (not tested), However, the PQ-SNARK aggregation is only "semi-finished"

# What is in the repository 
1. A Generalized XMSS implementation (untested)
2. A Basic Winternitz OTS Scheme
3. A Target-Sum Winternitz OTS Scheme
4. SHA messageHash, TweakHash, and PRF
5. Blake3 messageHash and TweakHash
6. An **unfinished** implmentation of a multiSignature dataStructure/class
7. A **semi-Finished** implementation of a R1CS constraint system for aggregating the multi signatures with Aurora PQ-SNARK.
8. A "Finished" Python version of the Signature Scheme. ( The XMSS is **allegedly** Finished and tested in the Python version, Testing was not official )

# Building
No concrete Build instructions have been made, However, here are the information needed to Build. 
1. Most Implementations (almost all) are in the headers, Therefore, to use, simply put the highest ranking header to use in a .cpp or .tcc, etc.
For example, to use the Winternitz OTS scheme, simply put ```#include "src/inc_encoding/basic_winternitz.hpp"``` in a .cpp or equivalent file.

2. When trying to compile the R1CS aggregate (if implementation is done) first compile the ```libiop``` submodule (look at their repository) and link it with the R1CS file.

# Files
File names are self explanatory
