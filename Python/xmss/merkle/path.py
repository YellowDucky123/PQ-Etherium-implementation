"""
Merkle Path Implementation

This module implements Merkle paths used in XMSS for authenticating leaf nodes.
"""

from typing import List, Any, Tuple
from ..hash_functions.tweakable import TweakableHash
import hashlib


class MerklePath:
    """
    Merkle Path implementation for XMSS.
    
    Represents an authentication path from a leaf node to the root of a Merkle tree.
    """
    
    def __init__(self, path_nodes: List[Tuple[Any, bool]], hash_function: TweakableHash):
        """
        Initialize a Merkle path.
        
        Args:
            path_nodes: List of (node_value, is_left_sibling) tuples representing the path
            hash_function: Tweakable hash function for path verification
        """
        self.path_nodes = path_nodes
        self.hash_function = hash_function or hashlib.sha256()
    
    def verify(self, leaf: Any, root: Any, leaf_index: int) -> bool:
        """
        Verify the Merkle path.
        
        Args:
            leaf: Leaf node to verify
            root: Expected root value
            leaf_index: Index of the leaf node
            
        Returns:
            True if path is valid, False otherwise
        """
        if not isinstance(leaf, bytes) or not isinstance(root, bytes):
            return False
            
        current_hash = leaf
        index = leaf_index
        
        # Walk up the tree using the path
        for node_value, is_left in self.path_nodes:
            if not isinstance(node_value, bytes):
                return False
                
            if index % 2 == 0:  # Current node is left
                current_hash = self._hash_with_direction(current_hash, node_value, index // 2)
            else:  # Current node is right
                current_hash = self._hash_with_direction(node_value, current_hash, index // 2)
            index //= 2
        
        # Check if we arrived at the expected root
        return current_hash == root
    
    def serialize(self) -> bytes:
        """
        Serialize the Merkle path to bytes.
        
        Returns:
            Serialized Merkle path
        """
        # Simple serialization: concatenate all path nodes
        # In a real implementation, this would be more sophisticated
        serialized = b""
        for node_value, is_left in self.path_nodes:
            # Add node value length and data
            serialized += len(node_value).to_bytes(4, 'big')
            serialized += node_value
            # Add direction bit
            serialized += b'\x01' if is_left else b'\x00'
        return serialized
    
    @classmethod
    def deserialize(cls, data: bytes) -> 'MerklePath':
        """
        Deserialize a Merkle path from bytes.
        
        Args:
            data: Serialized Merkle path
            
        Returns:
            Deserialized MerklePath object
        """
        # Simple deserialization - in a real implementation, this would be more sophisticated
        path_nodes = []
        offset = 0
        while offset < len(data):
            # Read node value length
            length = int.from_bytes(data[offset:offset+4], 'big')
            offset += 4
            # Read node value
            node_value = data[offset:offset+length]
            offset += length
            # Read direction bit
            is_left = data[offset] == 1
            offset += 1
            path_nodes.append((node_value, is_left))
        
        return cls(path_nodes, None)
    
    def _compute_root_from_path(self, leaf: Any, leaf_index: int) -> Any:
        """
        Compute the root value by following the authentication path.
        
        Args:
            leaf: Leaf node
            leaf_index: Index of the leaf node
            
        Returns:
            Computed root value
        """
        current_hash = leaf
        index = leaf_index
        
        # Walk up the tree using the path
        for node_value, is_left in self.path_nodes:
            if index % 2 == 0:  # Current node is left
                current_hash = self._hash_with_direction(current_hash, node_value, index // 2)
            else:  # Current node is right
                current_hash = self._hash_with_direction(node_value, current_hash, index // 2)
            index //= 2
        
        return current_hash
    
    def _hash_with_direction(self, left: Any, right: Any, level: int) -> Any:
        """
        Hash two nodes together with level information.
        
        Args:
            left: Left node
            right: Right node
            level: Level in the tree
            
        Returns:
            Hash of the two nodes
        """
        if self.hash_function and hasattr(self.hash_function, 'hash_with_tweak'):
            # Use tweakable hash if available
            tweak = level.to_bytes(4, 'big')
            return self.hash_function.hash_with_tweak(left + right, tweak)
        else:
            # Fallback to standard hash
            return hashlib.sha256(left + right).digest()