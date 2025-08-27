"""
Unit tests for Target-Sum Winternitz One-Time Signature scheme
"""

import unittest
from xmss.otss.target_sum_winternitz import TargetSumWinternitzOTS


class TestTargetSumWinternitzOTS(unittest.TestCase):
    
    def setUp(self):
        """Set up test fixtures before each test method."""
        self.test_message = b"Test message for Target-Sum Winternitz OTS"
        
    def test_initialization_valid_w_values(self):
        """Test TargetSumWinternitzOTS initialization with valid w values"""
        valid_w_values = [1, 2, 4, 8, 16]
        for w in valid_w_values:
            with self.subTest(w=w):
                ots = TargetSumWinternitzOTS(w=w)
                self.assertEqual(ots.w, w)
                
    def test_initialization_invalid_w_value(self):
        """Test that invalid w values raise ValueError"""
        with self.assertRaises(ValueError):
            TargetSumWinternitzOTS(w=3)  # Invalid w value
            
    def test_keypair_generation(self):
        """Test keypair generation"""
        ots = TargetSumWinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        self.assertIsNotNone(private_key)
        self.assertIsNotNone(public_key)
        self.assertEqual(len(private_key), len(public_key))
        self.assertGreater(len(private_key), 0)
        
    def test_sign_and_verify(self):
        """Test signing and verification"""
        ots = TargetSumWinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        # Sign a message
        signature = ots.sign(self.test_message, private_key)
        
        # Verify the signature
        is_valid = ots.verify(self.test_message, signature, public_key)
        self.assertTrue(is_valid)
        
    def test_verify_with_wrong_message(self):
        """Test that verification fails with wrong message"""
        ots = TargetSumWinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        # Sign a message
        signature = ots.sign(self.test_message, private_key)
        
        # Verify with wrong message should fail
        wrong_message = b"Wrong message"
        is_valid = ots.verify(wrong_message, signature, public_key)
        self.assertFalse(is_valid)
        
    def test_verify_with_wrong_signature(self):
        """Test that verification fails with wrong signature"""
        ots = TargetSumWinternitzOTS(w=4)
        private_key1, public_key1 = ots.generate_keypair()
        private_key2, public_key2 = ots.generate_keypair()
        
        # Sign with first keypair
        signature1 = ots.sign(self.test_message, private_key1)
        
        # Verify with second public key should fail
        is_valid = ots.verify(self.test_message, signature1, public_key2)
        self.assertFalse(is_valid)
        
    def test_different_w_values(self):
        """Test Target-Sum Winternitz OTS with different w values"""
        w_values = [2, 4, 8]
        for w in w_values:
            with self.subTest(w=w):
                ots = TargetSumWinternitzOTS(w=w)
                private_key, public_key = ots.generate_keypair()
                signature = ots.sign(self.test_message, private_key)
                is_valid = ots.verify(self.test_message, signature, public_key)
                self.assertTrue(is_valid, f"Verification failed for w={w}")
                
    def test_get_public_key_from_signature(self):
        """Test deriving public key from signature"""
        ots = TargetSumWinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        # Sign a message
        signature = ots.sign(self.test_message, private_key)
        
        # Derive public key from signature and message
        derived_public_key = ots.get_public_key_from_signature(self.test_message, signature)
        
        # Should match the original public key
        self.assertEqual(derived_public_key, public_key)
        
    def test_encode_message_with_target_sum(self):
        """Test message encoding with target sum"""
        ots = TargetSumWinternitzOTS(w=4)
        print(f"num_msg_blocks: {ots.num_msg_blocks}")
        print(f"target_sum: {ots.target_sum}")
        print(f"max_chain_len: {ots.max_chain_len}")
        encoded = ots._encode_message_with_target_sum(self.test_message)
        
        # Should return a list of integers
        self.assertIsInstance(encoded, list)
        print(f"Length of encoded: {len(encoded)}")
        for i, item in enumerate(encoded):
            self.assertIsInstance(item, int)
            print(f"Block {i}: {item}")
            
        # All values should be in valid range for w=4 (0-15)
        for item in encoded:
            self.assertGreaterEqual(item, 0)
            self.assertLess(item, 16)  # 2^w - 1 for w=4
            
        # Check that the sum matches target
        actual_sum = sum(encoded)
        self.assertEqual(actual_sum, ots.target_sum)
        print(f"Target sum: {ots.target_sum}, Actual sum: {actual_sum}, Encoded: {encoded}")
            
    def test_calculate_target_sum(self):
        """Test target sum computation"""
        ots = TargetSumWinternitzOTS(w=4)
        target_sum = ots._calculate_target_sum(10)
        
        self.assertIsInstance(target_sum, int)
        self.assertGreater(target_sum, 0)
        
    def test_chain_hash(self):
        """Test chain hashing function"""
        ots = TargetSumWinternitzOTS(w=4)
        start_value = b"test_start_value"
        chain_length = 5
        index = 0
        
        result = ots._chain_hash(start_value, chain_length, index)
        
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)  # SHA256 digest size


if __name__ == '__main__':
    unittest.main()