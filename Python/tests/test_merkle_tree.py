"""
Unit tests for Merkle tree implementation
"""

import unittest
import hashlib
from xmss.merkle.tree import MerkleTree, MerklePath


class TestMerkleTree(unittest.TestCase):
    
    def setUp(self):
        """Set up test fixtures before each test method."""
        self.test_leaves = [b"leaf0", b"leaf1", b"leaf2", b"leaf3"]
        
    def test_initialization(self):
        """Test Merkle tree initialization"""
        tree = MerkleTree(self.test_leaves)
        
        self.assertIsNotNone(tree)
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.leaves), 4)
        
    def test_empty_tree_initialization(self):
        """Test that empty tree raises ValueError"""
        with self.assertRaises(ValueError):
            MerkleTree([])
            
    def test_get_root(self):
        """Test getting the root of the tree"""
        tree = MerkleTree(self.test_leaves)
        root = tree.get_root()
        
        self.assertIsNotNone(root)
        self.assertIsInstance(root, bytes)
        
    def test_get_leaf(self):
        """Test getting a leaf by index"""
        tree = MerkleTree(self.test_leaves)
        
        for i, expected_leaf in enumerate(self.test_leaves):
            with self.subTest(i=i):
                leaf = tree.get_leaf(i)
                self.assertEqual(leaf, expected_leaf)
                
    def test_get_leaf_out_of_range(self):
        """Test that getting leaf out of range raises IndexError"""
        tree = MerkleTree(self.test_leaves)
        
        with self.assertRaises(IndexError):
            tree.get_leaf(-1)
            
        with self.assertRaises(IndexError):
            tree.get_leaf(len(self.test_leaves))
            
    def test_get_path(self):
        """Test getting authentication path"""
        tree = MerkleTree(self.test_leaves)
        
        for i in range(len(self.test_leaves)):
            with self.subTest(i=i):
                path = tree.get_path(i)
                self.assertIsInstance(path, MerklePath)
                self.assertIsNotNone(path.nodes)
                
    def test_get_path_out_of_range(self):
        """Test that getting path for invalid index raises IndexError"""
        tree = MerkleTree(self.test_leaves)
        
        with self.assertRaises(IndexError):
            tree.get_path(-1)
            
        with self.assertRaises(IndexError):
            tree.get_path(len(self.test_leaves))
            
    def test_verify_path(self):
        """Test path verification"""
        tree = MerkleTree(self.test_leaves)
        
        for i, leaf in enumerate(self.test_leaves):
            with self.subTest(i=i):
                path = tree.get_path(i)
                is_valid = tree.verify_path(leaf, path, i)
                self.assertTrue(is_valid)
                
    def test_verify_path_with_wrong_leaf(self):
        """Test that path verification fails with wrong leaf"""
        tree = MerkleTree(self.test_leaves)
        path = tree.get_path(0)
        
        wrong_leaf = b"wrong_leaf"
        is_valid = tree.verify_path(wrong_leaf, path, 0)
        self.assertFalse(is_valid)
        
    def test_odd_number_of_leaves(self):
        """Test tree construction with odd number of leaves"""
        odd_leaves = [b"leaf0", b"leaf1", b"leaf2"]
        tree = MerkleTree(odd_leaves)
        
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.leaves), 3)
        
        # Test paths for all leaves
        for i in range(len(odd_leaves)):
            path = tree.get_path(i)
            is_valid = tree.verify_path(odd_leaves[i], path, i)
            self.assertTrue(is_valid, f"Failed to verify path for leaf {i}")
            
    def test_single_leaf(self):
        """Test tree with single leaf"""
        single_leaf = [b"single_leaf"]
        tree = MerkleTree(single_leaf)
        
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.leaves), 1)
        
        path = tree.get_path(0)
        is_valid = tree.verify_path(single_leaf[0], path, 0)
        self.assertTrue(is_valid)
        
    def test_large_tree(self):
        """Test tree with larger number of leaves"""
        large_leaves = [f"leaf{i}".encode() for i in range(16)]
        tree = MerkleTree(large_leaves)
        
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.leaves), 16)
        
        # Test a few paths
        for i in [0, 7, 15]:
            path = tree.get_path(i)
            is_valid = tree.verify_path(large_leaves[i], path, i)
            self.assertTrue(is_valid)
            
    def test_custom_hash_function(self):
        """Test tree with custom hash function"""
        def custom_hash(data):
            return hashlib.sha512(data)
            
        tree = MerkleTree(self.test_leaves, hash_function=custom_hash)
        
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.root), 64)  # SHA512 digest size
        
    def test_merkle_path_verify(self):
        """Test MerklePath verification method directly"""
        tree = MerkleTree(self.test_leaves)
        leaf = self.test_leaves[0]
        path = tree.get_path(0)
        
        # Should verify correctly
        is_valid = path.verify(leaf, tree.root, 0)
        self.assertTrue(is_valid)
        
        # Should fail with wrong root
        is_valid = path.verify(leaf, b"wrong_root", 0)
        self.assertFalse(is_valid)
        
        # Should fail with wrong leaf
        is_valid = path.verify(b"wrong_leaf", tree.root, 0)
        self.assertFalse(is_valid)
        
        # Should fail with wrong index
        is_valid = path.verify(leaf, tree.root, 1)
        self.assertFalse(is_valid)
        
    def test_tree_with_duplicate_leaves(self):
        """Test tree with duplicate leaves"""
        duplicate_leaves = [b"same_leaf", b"same_leaf", b"same_leaf"]
        tree = MerkleTree(duplicate_leaves)
        
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.leaves), 3)
        
        # All paths should verify correctly
        for i in range(len(duplicate_leaves)):
            path = tree.get_path(i)
            is_valid = tree.verify_path(duplicate_leaves[i], path, i)
            self.assertTrue(is_valid)
            
    def test_tree_with_empty_leaf(self):
        """Test tree with empty leaf values"""
        leaves_with_empty = [b"", b"leaf1", b"leaf2"]
        tree = MerkleTree(leaves_with_empty)
        
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.leaves), 3)
        
        # All paths should verify correctly
        for i in range(len(leaves_with_empty)):
            path = tree.get_path(i)
            is_valid = tree.verify_path(leaves_with_empty[i], path, i)
            self.assertTrue(is_valid)
            
    def test_tree_with_long_leaves(self):
        """Test tree with long leaf values"""
        long_leaves = [b"A" * 1000, b"B" * 1000, b"C" * 1000]
        tree = MerkleTree(long_leaves)
        
        self.assertIsNotNone(tree.root)
        self.assertEqual(len(tree.leaves), 3)
        
        # All paths should verify correctly
        for i in range(len(long_leaves)):
            path = tree.get_path(i)
            is_valid = tree.verify_path(long_leaves[i], path, i)
            self.assertTrue(is_valid)


if __name__ == '__main__':
    unittest.main()