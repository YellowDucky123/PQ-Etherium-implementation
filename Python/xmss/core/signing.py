"""
Signing Module for XMSS

This module implements the signing algorithms for the XMSS signature scheme.
"""

from typing import Any
from ..otss.base import OTS
from ..merkle.tree import MerkleTree
from dataclasses import dataclass
from typing import Tuple
from .keygen import XMSSPrivateKey


@dataclass
class XMSSSignature:
    """XMSS Signature structure"""
    ots_signature: Any  # One-time signature
    ots_public_key: Any  # One-time public key
    auth_path: Any  # Authentication path in Merkle tree
    index: int  # Index of the key used


def sign_message(message: bytes, private_key: XMSSPrivateKey) -> XMSSSignature:
    """
    Sign a message using the XMSS signature scheme.
    
    To sign the i-th message, the signer uses the i-th one-time private key 
    to create a one-time signature, includes the corresponding one-time 
    public key, and includes the Merkle path from that public key to the root.
    
    Args:
        message: Message to sign
        private_key: XMSS private key containing one-time private keys
        
    Returns:
        XMSS signature containing one-time signature, one-time public key, and Merkle path
    """
    # Check if we still have unused keys
    if private_key.index >= len(private_key.ots_keypairs):
        raise ValueError("No more one-time signatures available")
    
    # Get the one-time key pair for current index
    ots_private_key, ots_public_key = private_key.ots_keypairs[private_key.index]
    
    # Generate one-time signature
    ots_signature = _generate_ots_signature(message, ots_private_key, private_key.ots_scheme)
    
    # Get Merkle authentication path
    auth_path = _get_merkle_path(private_key.index, private_key.merkle_tree)
    
    # Create signature
    signature = XMSSSignature(
        ots_signature=ots_signature,
        ots_public_key=ots_public_key,
        auth_path=auth_path,
        index=private_key.index
    )
    
    # Update private key state
    _update_signer_state(private_key)
    
    return signature


def _generate_ots_signature(message: bytes, ots_private_key: Any, ots_scheme: OTS) -> Any:
    """
    Generate a one-time signature for a message.
    
    Args:
        message: Message to sign
        ots_private_key: One-time signature private key
        ots_scheme: One-time signature scheme to use
        
    Returns:
        One-time signature
    """
    return ots_scheme.sign(message, ots_private_key)


def _get_merkle_path(index: int, merkle_tree: MerkleTree) -> Any:
    """
    Get the Merkle path from a leaf to the root.
    
    Args:
        index: Index of the leaf node
        merkle_tree: Merkle tree to get path from
        
    Returns:
        Authentication path from leaf to root
    """
    return merkle_tree.get_path(index) if hasattr(merkle_tree, 'get_path') else f"path_{index}"


def _update_signer_state(private_key: XMSSPrivateKey) -> None:
    """
    Update the signer's state after generating a signature.
    
    Important for stateful signature schemes to prevent key reuse.
    
    Args:
        private_key: Current private key to update
    """
    private_key.index += 1