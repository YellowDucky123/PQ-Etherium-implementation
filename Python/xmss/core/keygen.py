"""
Key Generation Module for XMSS

This module implements key generation algorithms for the XMSS signature scheme.
"""

from typing import Tuple, Any, List
from ..otss.base import OTS
from ..merkle.tree import MerkleTree
from dataclasses import dataclass
from typing import Optional
import hashlib


@dataclass
class XMSSPublicKey:
    """XMSS Public Key - the root of the Merkle tree"""
    root: bytes
    height: int
    ots_scheme_params: dict


@dataclass
class XMSSPrivateKey:
    """XMSS Private Key - contains all OTS key pairs and state information"""
    ots_keypairs: List[Tuple[Any, Any]]  # List of (private, public) key pairs
    merkle_tree: MerkleTree
    index: int  # Current index (state)
    height: int
    ots_scheme: OTS


def generate_xmss_keypair(ots_scheme: OTS, height: int) -> Tuple[XMSSPrivateKey, XMSSPublicKey]:
    """
    Generate an XMSS key pair.
    
    This function generates a long list of one-time key pairs, organizes them
    in a Merkle tree, and uses the Merkle root as the public key.
    
    Args:
        ots_scheme: One-time signature scheme to use
        height: Height of the Merkle tree (determines number of signatures: 2^height)
        
    Returns:
        Tuple of (private_key, public_key) for the XMSS scheme
    """
    # Generate 2^height one-time key pairs
    num_keys = 2 ** height
    ots_keypairs = _generate_ots_keypairs(ots_scheme, num_keys)
    
    # Extract public keys for Merkle tree construction
    public_keys = [pub_key for _, pub_key in ots_keypairs]
    
    # Build Merkle tree
    merkle_tree = _build_merkle_tree(public_keys)
    
    # Compute root (public key)
    root = _compute_tree_root(merkle_tree)
    
    # Create private key with all key pairs and initial state (index=0)
    private_key = XMSSPrivateKey(
        ots_keypairs=ots_keypairs,
        merkle_tree=merkle_tree,
        index=0,
        height=height,
        ots_scheme=ots_scheme
    )
    
    # Create public key
    public_key = XMSSPublicKey(
        root=root,
        height=height,
        ots_scheme_params={}  # Would contain OTS scheme parameters in full implementation
    )
    
    return private_key, public_key


def _generate_ots_keypairs(ots_scheme: OTS, count: int) -> List[Tuple[Any, Any]]:
    """
    Generate a list of one-time signature key pairs.
    
    Args:
        ots_scheme: One-time signature scheme to use
        count: Number of key pairs to generate
        
    Returns:
        List of (private_key, public_key) tuples
    """
    keypairs = []
    for _ in range(count):
        keypair = ots_scheme.generate_keypair()
        keypairs.append(keypair)
    return keypairs


def _build_merkle_tree(public_keys: List[Any]) -> MerkleTree:
    """
    Build a Merkle tree from a list of public keys.
    
    Args:
        public_keys: List of one-time public keys
        
    Returns:
        Constructed Merkle tree
    """
    # Convert public keys to bytes using a consistent serialization method
    bytes_keys = []
    for key in public_keys:
        if isinstance(key, (list, tuple)):
            # If it's a structured key (e.g., list/tuple of bytes), serialize it using a canonical method
            serialized = b''.join(item if isinstance(item, bytes) else str(item).encode() for item in key)
            bytes_keys.append(hashlib.sha256(serialized).digest())
        elif isinstance(key, bytes):
            bytes_keys.append(key)
        else:
            # Fallback for other types: use a consistent string representation
            bytes_keys.append(str(key).encode())
    
    return MerkleTree(bytes_keys)


def _compute_tree_root(merkle_tree: MerkleTree) -> bytes:
    """
    Compute the root of the Merkle tree.
    
    Args:
        merkle_tree: Merkle tree to compute root for
        
    Returns:
        Root of the Merkle tree (serves as XMSS public key)
    """
    root = merkle_tree.get_root()
    if not isinstance(root, bytes):
        raise ValueError("Merkle tree root must be a bytes object")
    return root