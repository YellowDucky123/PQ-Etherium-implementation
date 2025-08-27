"""
XMSS-based Post-Quantum Signature Scheme Package

This package implements the XMSS (eXtended Merkle Signature Scheme) variants 
for post-quantum blockchain security as described in the paper "Hash-Based 
Post-Quantum Signatures: A Study of XMSS Variants".

The implementation focuses on XMSS variants that can serve as post-quantum 
alternatives to BLS signatures, with efficient signature aggregation 
capabilities using pqSNARKs.
"""

# Version of the package
__version__ = "1.0.0"

# Import main components for easier access
from .core.xmss import XMSS
from .core.keygen import generate_xmss_keypair
from .core.signing import sign_message
from .core.verification import verify_signature

# Import OTS schemes
from .otss.winternitz import WinternitzOTS
from .otss.target_sum_winternitz import TargetSumWinternitzOTS

# Import Merkle tree components
from .merkle.tree import MerkleTree

# Import encoding schemes
from .encoding.incomparable import IncomparableEncoding

# Import hash functions
from .hash_functions.tweakable import TweakableHash

__all__ = [
    "XMSS",
    "generate_xmss_keypair",
    "sign_message",
    "verify_signature",
    "WinternitzOTS",
    "TargetSumWinternitzOTS",
    "MerkleTree",
    "IncomparableEncoding",
    "TweakableHash"
]