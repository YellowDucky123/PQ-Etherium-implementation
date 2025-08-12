"""
Verification Module for XMSS

This module implements the verification algorithms for the XMSS signature scheme.
"""

from typing import Any
from ..merkle.tree import MerkleTree
from .xmss import XMSSSignature


def verify_signature(message: bytes, signature: XMSSSignature, public_key: Any, ots_scheme: Any) -> bool:
    """
    Verify an XMSS signature.
    
    This function verifies both the one-time signature and the Merkle authentication path.
    
    Args:
        message: Message that was signed
        signature: XMSS signature to verify
        public_key: XMSS public key
        ots_scheme: One-time signature scheme used
        
    Returns:
        True if signature is valid, False otherwise
    """
    try:
        # Verify the one-time signature
        ots_valid = ots_scheme.verify(message, signature.ots_signature, signature.ots_public_key)
        if not ots_valid:
            return False
        
        # Hash the one-time public key to get the Merkle tree leaf
        if isinstance(signature.ots_public_key, list):
            # If it's a list of bytes (like Winternitz), hash them together
            ots_pubkey_hash = ots_scheme.hash_function(b''.join(signature.ots_public_key)).digest()
        else:
            # If it's already bytes
            ots_pubkey_hash = signature.ots_public_key
        
        # Verify the Merkle authentication path
        path_valid = signature.auth_path.verify(ots_pubkey_hash, public_key.root, signature.index)
        
        return path_valid
    except Exception:
        # Any exception during verification means the signature is invalid
        return False


