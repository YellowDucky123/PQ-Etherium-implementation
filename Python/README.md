# XMSS Post-Quantum Signature Scheme

## Overview

This Python implements the eXtended Merkle Signature Scheme (XMSS), a post-quantum cryptographic signature scheme based on hash functions. XMSS is a stateful hash-based signature scheme that provides security against attacks using quantum computers.

The implementation follows the RFC 8391 specification and includes:
- Winternitz One-Time Signature (WOTS) scheme
- Merkle hash trees for key commitment
- Incomparable encoding techniques
- Support for various hash functions

## Features

- **Post-Quantum Security**: Based on the security of underlying hash functions, which are believed to be quantum-resistant
- **Stateful Signatures**: Efficient multiple-time signature scheme using Merkle trees
- **RFC 8391 Compliant**: Follows the official XMSS standard
- **Modular Design**: Clean separation of core components
- **Extensible Architecture**: Easy to add new hash functions or OTS schemes

## Installation

```bash
pip install -e .
```

## Usage

A basic example of using the XMSS signature scheme:

```python
from xmss.core.xmss import XMSS
from xmss.otss.winternitz import WinternitzOTS

# Create XMSS instance with Winternitz OTS and tree height of 10
ots_scheme = WinternitzOTS(w=4)
xmss = XMSS(ots_scheme=ots_scheme, height=10)

# Generate keypair
private_key, public_key = xmss.generate_keypair()

# Sign a message
message = b"Hello, XMSS!"
signature = xmss.sign(message)

# Verify signature
is_valid = xmss.verify(message, signature)
print(f"Signature verification: {'PASSED' if is_valid else 'FAILED'}")
```

## Components

### Core XMSS Implementation
- `xmss/core/xmss.py`: Main XMSS class implementing key generation, signing, and verification
- `xmss/core/keygen.py`: Key generation algorithms
- `xmss/core/signing.py`: Signing algorithms
- `xmss/core/verification.py`: Signature verification algorithms

### One-Time Signature Schemes
- `xmss/otss/winternitz.py`: Winternitz One-Time Signature scheme
- `xmss/otss/target_sum_winternitz.py`: Target-Sum Winternitz scheme
- `xmss/otss/base.py`: Abstract base class for OTS schemes

### Merkle Tree
- `xmss/merkle/tree.py`: Merkle tree implementation
- `xmss/merkle/path.py`: Merkle authentication paths

### Encoding
- `xmss/encoding/incomparable.py`: Incomparable encoding techniques
- `xmss/encoding/base.py`: Abstract base class for encodings

### Hash Functions
- `xmss/hash_functions/tweakable.py`: Tweakable hash functions
- `xmss/hash_functions/sha3.py`: SHA-3 hash functions
- `xmss/hash_functions/poseidon.py`: Poseidon hash functions (for zero-knowledge proofs)

## Security

This implementation is designed with security in mind:
- Uses cryptographically secure random number generation
- Follows the RFC 8391 specification
- Includes proper state management to prevent key reuse
- Provides resistance against side-channel attacks

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## References

1. RFC 8391 - XMSS: eXtended Merkle Signature Scheme
2. "Hash-Based Signatures: An Update" by Johannes Buchmann et al.
3. "XMSS -- A Practical Forward Secure Signature Scheme Based on Minimal Security Assumptions" by Buchmann, Döring, and Dahmen.
