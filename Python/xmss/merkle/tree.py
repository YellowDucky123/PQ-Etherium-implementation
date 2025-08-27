"""
Merkle Tree Implementation

This module implements Merkle trees used in XMSS to commit to one-time public keys.
"""

from typing import List, Any
from dataclasses import dataclass
import hashlib


@dataclass
class MerklePathNode:
    """Represents a node in a Merkle path"""
    value: bytes
    is_left: bool  # True if this is a left sibling, False if right sibling


@dataclass
class MerklePath:
    """Represents a Merkle authentication path"""
    nodes: List[MerklePathNode]
    
    def verify(self, leaf: bytes, root: bytes, index: int) -> bool:
        """Verify the Merkle path"""
        # Start with the leaf
        current_hash = leaf
        path_index = index
        
        # Walk up the tree
        for node in self.nodes:
            if path_index % 2 == 0:  # Current node is left
                current_hash = self._hash_pair(current_hash, node.value)
            else:  # Current node is right
                current_hash = self._hash_pair(node.value, current_hash)
            path_index //= 2
        
        # Check if we arrived at the expected root
        return current_hash == root
    
    def _hash_pair(self, left: bytes, right: bytes) -> bytes:
        """Hash a pair of nodes together"""
        return hashlib.sha256(left + right).digest()


class MerkleTree:
    """
    Merkle Tree implementation for XMSS.
    
    Used to commit to a long list of one-time public keys, with the Merkle root
    serving as the XMSS public key.
    """
    
    def __init__(self, leaf_nodes: List[Any], hash_function=None):
        """
        Initialize and construct a Merkle tree.
        
        Args:
            leaf_nodes: List of leaf nodes (one-time public keys). Each leaf will be
                        converted to bytes if it isn't already.
            hash_function: Hash function to use for combining nodes. Must be a function
                           that takes input bytes and has digest() method. 
                           Default: SHA256
                           
        Raises:
            ValueError: If leaf_nodes is None or empty
        """
        if not leaf_nodes:
            raise ValueError("MerkleTree requires at least one leaf node")
            
        self.hash_function = hash_function or hashlib.sha256
        self.leaves = [leaf if isinstance(leaf, bytes) else str(leaf).encode() for leaf in leaf_nodes]
        
        # Validate that all leaves are now bytes
        for i, leaf in enumerate(self.leaves):
            if not isinstance(leaf, bytes):
                raise ValueError(f"Leaf at index {i} is not bytes: {leaf}")
                
        self.tree = self._build_tree(self.leaves)
        self.root = self.tree[-1][0] if self.tree else None  # Root is the last level
    
    def get_root(self) -> bytes:
        """
        Get the root of the Merkle tree.
        
        Returns:
            Root node of the Merkle tree
        """
        return self.root
    
    def get_leaf(self, index: int) -> bytes:
        """
        Get a leaf node at a specific index.
        
        Args:
            index: Index of the leaf node to retrieve
            
        Returns:
            Leaf node at the specified index
            
        Raises:
            IndexError: If index is out of range
        """
        if not (0 <= index < len(self.leaves)):
            raise IndexError(f"Leaf index {index} out of range [0, {len(self.leaves) - 1}]")
        return self.leaves[index]
    
    def get_path(self, index: int) -> MerklePath:
        """
        Get the authentication path from a leaf to the root.
        
        For each level of the tree, we collect the sibling node needed to compute
        the parent hash, all the way up to the root.
        
        Args:
            index: Index of the leaf node
            
        Returns:
            Authentication path from leaf to root
            
        Raises:
            IndexError: If index is out of range
        """
        if not (0 <= index < len(self.leaves)):
            raise IndexError(f"Leaf index {index} out of range [0, {len(self.leaves) - 1}]")
        
        path_nodes = []
        path_index = index
        
        # Walk up the tree, collecting siblings
        for level_index in range(len(self.tree) - 1):  # Exclude the root level
            level = self.tree[level_index]
            sibling_index = path_index ^ 1  # XOR to get sibling index
            
            # Check if sibling exists (handle odd number of nodes)
            if sibling_index < len(level):
                # Get the sibling value
                sibling_value = level[sibling_index]
                is_left = (sibling_index % 2 == 0)
                path_nodes.append(MerklePathNode(sibling_value, is_left))
            else:
                # If there's no sibling (odd node), use the node itself as sibling
                node_value = level[path_index]
                is_left = (path_index % 2 == 0)
                path_nodes.append(MerklePathNode(node_value, is_left))
            
            path_index //= 2  # Move to parent index
        
        return MerklePath(path_nodes)
    
    def verify_path(self, leaf: bytes, path: MerklePath, index: int) -> bool:
        """
        Verify an authentication path for a leaf node.
        
        Args:
            leaf: Leaf node to verify
            path: Authentication path
            index: Index of the leaf node
            
        Returns:
            True if path is valid, False otherwise
            
        Raises:
            ValueError: If inputs are invalid
        """
        return path.verify(leaf, self.root, index)
    
    def _build_tree(self, leaves: List[bytes]) -> List[List[bytes]]:
        """
        Build the complete Merkle tree from leaf nodes.
        
        Args:
            leaves: List of leaf nodes
            
        Returns:
            Complete tree structure as list of levels
        """
        if not leaves:
            return []
        
        # Start with leaves as the first level
        tree = [leaves[:]]  # Copy the leaves
        current_level = leaves
        
        # Build the tree level by level
        while len(current_level) > 1:
            next_level = []
            for i in range(0, len(current_level), 2):
                left = current_level[i]
                # If odd number of nodes, duplicate the last one
                right = current_level[i + 1] if i + 1 < len(current_level) else left
                # Hash the pair
                combined = left + right
                next_level.append(self.hash_function(combined).digest())
            tree.append(next_level)
            current_level = next_level
        
        return tree