"""
Unit tests for Winternitz One-Time Signature scheme
"""

import unittest
from xmss.otss.winternitz import WinternitzOTS


class TestWinternitzOTS(unittest.TestCase):
    
    def setUp(self):
        """Set up test fixtures before each test method."""
        self.test_message = b"Test message for Winternitz OTS"
        
    def test_initialization_valid_w_values(self):
        """Test WinternitzOTS initialization with valid w values"""
        valid_w_values = [1, 2, 4, 8, 16]
        for w in valid_w_values:
            with self.subTest(w=w):
                ots = WinternitzOTS(w=w)
                self.assertEqual(ots.w, w)
                
    def test_initialization_invalid_w_value(self):
        """Test that invalid w values raise ValueError"""
        with self.assertRaises(ValueError):
            WinternitzOTS(w=3)  # Invalid w value
            
    def test_keypair_generation(self):
        """Test keypair generation"""
        ots = WinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        self.assertIsNotNone(private_key)
        self.assertIsNotNone(public_key)
        self.assertEqual(len(private_key), len(public_key))
        self.assertGreater(len(private_key), 0)
        
    def test_sign_and_verify(self):
        """Test signing and verification"""
        ots = WinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        # Sign a message
        signature = ots.sign(self.test_message, private_key)
        
        # Verify the signature
        is_valid = ots.verify(self.test_message, signature, public_key)
        self.assertTrue(is_valid)
        
    def test_verify_with_wrong_message(self):
        """Test that verification fails with wrong message"""
        ots = WinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        # Sign a message
        signature = ots.sign(self.test_message, private_key)
        
        # Verify with wrong message should fail
        wrong_message = b"Wrong message"
        is_valid = ots.verify(wrong_message, signature, public_key)
        self.assertFalse(is_valid)
        
    def test_verify_with_wrong_signature(self):
        """Test that verification fails with wrong signature"""
        ots = WinternitzOTS(w=4)
        private_key1, public_key1 = ots.generate_keypair()
        private_key2, public_key2 = ots.generate_keypair()
        
        # Sign with first keypair
        signature1 = ots.sign(self.test_message, private_key1)
        
        # Verify with second public key should fail
        is_valid = ots.verify(self.test_message, signature1, public_key2)
        self.assertFalse(is_valid)
        
    def test_different_w_values(self):
        """Test Winternitz OTS with different w values"""
        w_values = [2, 4, 8]
        for w in w_values:
            with self.subTest(w=w):
                ots = WinternitzOTS(w=w)
                private_key, public_key = ots.generate_keypair()
                signature = ots.sign(self.test_message, private_key)
                is_valid = ots.verify(self.test_message, signature, public_key)
                self.assertTrue(is_valid, f"Verification failed for w={w}")
                
    def test_get_public_key_from_signature(self):
        """Test deriving public key from signature"""
        ots = WinternitzOTS(w=4)
        private_key, public_key = ots.generate_keypair()
        
        # Sign a message
        signature = ots.sign(self.test_message, private_key)
        
        # Derive public key from signature and message
        derived_public_key = ots.get_public_key_from_signature(self.test_message, signature)
        
        # Should match the original public key
        self.assertEqual(derived_public_key, public_key)
        
    def test_encode_message_with_checksum(self):
        """Test message encoding with checksum"""
        ots = WinternitzOTS(w=4)
        encoded = ots._encode_message_with_checksum(self.test_message)
        
        # Should return a list of integers
        self.assertIsInstance(encoded, list)
        for item in encoded:
            self.assertIsInstance(item, int)
            
        # All values should be in valid range for w=4 (0-15)
        for item in encoded:
            self.assertGreaterEqual(item, 0)
            self.assertLess(item, 16)  # 2^w - 1 for w=4
            
    def test_compute_checksum(self):
        """Test checksum computation"""
        ots = WinternitzOTS(w=4)
        # Create some dummy message blocks
        message_blocks = [1, 2, 3, 4, 5]
        checksum_blocks = ots._compute_checksum(message_blocks)
        
        self.assertIsInstance(checksum_blocks, list)
        self.assertGreater(len(checksum_blocks), 0)
        
    def test_chain_hash(self):
        """Test chain hashing function"""
        ots = WinternitzOTS(w=4)
        start_value = b"test_start_value"
        chain_length = 5
        index = 0
        
        result = ots._chain_hash(start_value, chain_length, index)
        
        self.assertIsInstance(result, bytes)
        self.assertEqual(len(result), 32)  # SHA256 digest size


if __name__ == '__main__':
    unittest.main()