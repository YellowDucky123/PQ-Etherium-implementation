"""
Unit tests for hash functions
"""

import unittest
import hashlib
from xmss.hash_functions.tweakable import TweakableHash
from xmss.hash_functions.sha3 import SHA3Hash
from xmss.hash_functions.poseidon import PoseidonHash


class TestTweakableHash(unittest.TestCase):
    """Test the abstract TweakableHash class through a concrete implementation"""
    
    def setUp(self):
        """Set up test fixtures"""
        # Create a simple concrete implementation for testing
        class ConcreteTweakableHash(TweakableHash):
            def __init__(self):
                self.output_len = 32
            
            def hash(self, data, tweak=None):
                if tweak is not None:
                    data = tweak + data
                hasher = hashlib.sha256()
                hasher.update(data)
                return hasher.digest()
            
            def get_output_length(self):
                return self.output_len
        
        self.hash_func = ConcreteTweakableHash()
        self.test_data = b"test data for hashing"
    
    def test_message_hash(self):
        """Test message hash function"""
        result = self.hash_func.message_hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), self.hash_func.get_output_length())
    
    def test_chain_hash(self):
        """Test chain hash function"""
        result = self.hash_func.chain_hash(self.test_data, 5)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), self.hash_func.get_output_length())
    
    def test_leaf_hash(self):
        """Test leaf hash function"""
        result = self.hash_func.leaf_hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), self.hash_func.get_output_length())
    
    def test_node_hash(self):
        """Test node hash function"""
        left_data = b"left node"
        right_data = b"right node"
        result = self.hash_func.node_hash(left_data, right_data, 3, 7)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), self.hash_func.get_output_length())


class TestSHA3Hash(unittest.TestCase):
    """Test SHA3 hash function implementation"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.sha3_256 = SHA3Hash("SHA3-256")
        self.sha3_512 = SHA3Hash("SHA3-512")
        self.test_data = b"test data for SHA3 hashing"
    
    def test_initialization(self):
        """Test SHA3Hash initialization"""
        self.assertEqual(self.sha3_256.get_output_length(), 32)
        self.assertEqual(self.sha3_512.get_output_length(), 64)
        
        with self.assertRaises(ValueError):
            SHA3Hash("SHA3-128")  # Unsupported variant
    
    def test_hash_function(self):
        """Test basic hash function"""
        result = self.sha3_256.hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
        
        result2 = self.sha3_512.hash(self.test_data)
        self.assertIsInstance(result2, bytes)
        self.assertEqual(len(result2), 64)
    
    def test_hash_with_tweak(self):
        """Test hash function with tweak"""
        tweak = b"test_tweak"
        result = self.sha3_256.hash(self.test_data, tweak)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_message_hash(self):
        """Test message hash function"""
        result = self.sha3_256.message_hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_chain_hash(self):
        """Test chain hash function"""
        result = self.sha3_256.chain_hash(self.test_data, 10)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_leaf_hash(self):
        """Test leaf hash function"""
        result = self.sha3_256.leaf_hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_node_hash(self):
        """Test node hash function"""
        left_data = b"left node"
        right_data = b"right node"
        result = self.sha3_256.node_hash(left_data, right_data, 2, 5)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)


class TestPoseidonHash(unittest.TestCase):
    """Test Poseidon hash function implementation"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.poseidon = PoseidonHash(field_size=2**64, rate=3, capacity=5, full_rounds=8, partial_rounds=56)
        self.test_data = b"test data for Poseidon hashing"
    
    def test_initialization(self):
        """Test PoseidonHash initialization"""
        self.assertEqual(self.poseidon.get_output_length(), 3)
    
    def test_hash_function(self):
        """Test basic hash function"""
        result = self.poseidon.hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)  # Using SHA256 as simulation
    
    def test_hash_with_tweak(self):
        """Test hash function with tweak"""
        tweak = b"test_tweak"
        result = self.poseidon.hash(self.test_data, tweak)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_message_hash(self):
        """Test message hash function"""
        result = self.poseidon.message_hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_chain_hash(self):
        """Test chain hash function"""
        result = self.poseidon.chain_hash(self.test_data, 7)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_leaf_hash(self):
        """Test leaf hash function"""
        result = self.poseidon.leaf_hash(self.test_data)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)
    
    def test_node_hash(self):
        """Test node hash function"""
        left_data = b"left node"
        right_data = b"right node"
        result = self.poseidon.node_hash(left_data, right_data, 4, 9)
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)


if __name__ == '__main__':
    unittest.main()