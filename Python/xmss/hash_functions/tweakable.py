"""
Tweakable Hash Function Implementation

This module implements tweakable hash functions used in XMSS for different
purposes such as message hashing, chain hashing, leaf hashing, and tree hashing.
"""

from abc import ABC, abstractmethod
from typing import Any, Optional
import hashlib


class TweakableHash(ABC):
    """
    Abstract base class for tweakable hash functions.
    
    Provides different hash instances for different purposes by using tweaks.
    """
    
    @abstractmethod
    def hash(self, data: bytes, tweak: Optional[bytes] = None) -> bytes:
        """
        Hash data with an optional tweak.
        
        Args:
            data: Data to hash
            tweak: Optional tweak to modify the hash function
            
        Returns:
            Hash of the data
        """
        pass
    
    @abstractmethod
    def get_output_length(self) -> int:
        """
        Get the output length of the hash function in bytes.
        
        Returns:
            Output length in bytes
        """
        pass
    
    def message_hash(self, data: bytes) -> bytes:
        """
        Hash function for message hashing.
        
        Args:
            data: Message data to hash
            
        Returns:
            Hash of the message
        """
        return self.hash(data, b"MSG")
    
    def chain_hash(self, data: bytes, index: int) -> bytes:
        """
        Hash function for chain hashing in Winternitz schemes.
        
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
        Hash function for leaf node hashing in Merkle trees.
        
        Args:
            data: Leaf data to hash
            
        Returns:
            Hash of the leaf data
        """
        return self.hash(data, b"LEAF")
    
    def node_hash(self, left: bytes, right: bytes, level: int, index: int) -> bytes:
        """
        Hash function for internal node hashing in Merkle trees.
        
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