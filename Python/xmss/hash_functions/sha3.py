"""
SHA-3 Hash Function Implementation

This module implements SHA-3 based hash functions for use in XMSS.
"""

from typing import Any, Optional
import hashlib
from .tweakable import TweakableHash


class SHA3Hash(TweakableHash):
    """
    SHA-3 based tweakable hash function.
    
    Supports different SHA-3 variants (SHA3-256, SHA3-512, etc.) with
    tweakable functionality for different XMSS purposes.
    """
    
    def __init__(self, variant: str = "SHA3-256"):
        """
        Initialize the SHA-3 hash function.
        
        Args:
            variant: SHA-3 variant to use (e.g., "SHA3-256", "SHA3-512")
        """
        self.variant = variant
        if variant == "SHA3-256":
            self.hash_func = hashlib.sha3_256
            self.output_length = 32
        elif variant == "SHA3-512":
            self.hash_func = hashlib.sha3_512
            self.output_length = 64
        else:
            raise ValueError(f"Unsupported SHA-3 variant: {variant}")
    
    def hash(self, data: bytes, tweak: Optional[bytes] = None) -> bytes:
        """
        Hash data with an optional tweak using SHA-3.
        
        Args:
            data: Data to hash
            tweak: Optional tweak to modify the hash function
            
        Returns:
            Hash of the data
        """
        if tweak is not None:
            data = self._apply_tweak(data, tweak)
        hasher = self.hash_func()
        hasher.update(data)
        return hasher.digest()
    
    def get_output_length(self) -> int:
        """
        Get the output length of the SHA-3 hash function in bytes.
        
        Returns:
            Output length in bytes
        """
        return self.output_length
    
    def message_hash(self, data: bytes) -> bytes:
        """
        Hash function for message hashing using SHA-3.
        
        Args:
            data: Message data to hash
            
        Returns:
            Hash of the message
        """
        return self.hash(data, b"MSG")
    
    def chain_hash(self, data: bytes, index: int) -> bytes:
        """
        Hash function for chain hashing in Winternitz schemes using SHA-3.
        
        Args:
            data: Data to hash
            index: Index for the chain hash
            
        Returns:
            Hash result
        """
        tweak = b"CHAIN" + index.to_bytes(4, byteorder='big')
        return self.hash(data, tweak)
    
    def leaf_hash(self, data: bytes) -> bytes:
        """
        Hash function for leaf node hashing in Merkle trees using SHA-3.
        
        Args:
            data: Leaf data to hash
            
        Returns:
            Hash of the leaf data
        """
        return self.hash(data, b"LEAF")
    
    def node_hash(self, left: bytes, right: bytes, level: int, index: int) -> bytes:
        """
        Hash function for internal node hashing in Merkle trees using SHA-3.
        
        Args:
            left: Left child node
            right: Right child node
            level: Level in the tree
            index: Index within the level
            
        Returns:
            Hash of the two child nodes
        """
        tweak = b"NODE" + level.to_bytes(4, byteorder='big') + index.to_bytes(4, byteorder='big')
        return self.hash(left + right, tweak)
    
    def _apply_tweak(self, data: bytes, tweak: bytes) -> bytes:
        """
        Apply a tweak to the data before hashing.
        
        Args:
            data: Original data
            tweak: Tweak to apply
            
        Returns:
            Tweaked data
        """
        return tweak + data