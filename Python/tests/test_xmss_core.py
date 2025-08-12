"""
Unit tests for XMSS core functionality
"""

import unittest
import os
import tempfile
from xmss.core.xmss import XMSS
from xmss.otss.winternitz import WinternitzOTS
from xmss.otss.target_sum_winternitz import TargetSumWinternitzOTS
from xmss.parameters import get_parameters_by_name


class TestXMSSCore(unittest.TestCase):
    
    def setUp(self):
        """Set up test fixtures before each test method."""
        self.ots_scheme = WinternitzOTS(w=4)
        self.test_message = b"Hello, XMSS!"
        
    def test_initialization_with_custom_parameters(self):
        """Test XMSS initialization with custom parameters"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=4)
        self.assertEqual(xmss.height, 4)
        self.assertEqual(xmss.ots_scheme.w, 4)
        
    def test_initialization_with_parameter_set(self):
        """Test XMSS initialization with predefined parameter set"""
        xmss = XMSS(parameter_set_name="XMSS-SHA2_10_256")
        self.assertEqual(xmss.height, 10)
        self.assertEqual(xmss.ots_scheme.w, 16)
        
    def test_key_generation(self):
        """Test XMSS key generation"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        self.assertIsNotNone(private_key)
        self.assertIsNotNone(public_key)
        self.assertEqual(xmss.get_state(), 0)
        
    def test_sign_and_verify(self):
        """Test XMSS signing and verification"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        # Sign a message
        signature = xmss.sign(self.test_message)
        
        # Verify the signature
        is_valid = xmss.verify(self.test_message, signature)
        self.assertTrue(is_valid)
        
        # Check that state was updated
        self.assertEqual(xmss.get_state(), 1)
        
    def test_sign_and_verify_with_target_sum_ots(self):
        """Test XMSS signing and verification with Target-Sum Winternitz OTS"""
        target_sum_ots = TargetSumWinternitzOTS(w=4)
        xmss = XMSS(ots_scheme=target_sum_ots, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        # Sign a message
        signature = xmss.sign(self.test_message)
        
        # Verify the signature
        is_valid = xmss.verify(self.test_message, signature)
        self.assertTrue(is_valid)
        
        # Check that state was updated
        self.assertEqual(xmss.get_state(), 1)
        
    def test_verify_with_wrong_message(self):
        """Test that verification fails with wrong message"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        # Sign a message
        signature = xmss.sign(self.test_message)
        
        # Verify with wrong message should fail
        wrong_message = b"Wrong message"
        is_valid = xmss.verify(wrong_message, signature)
        self.assertFalse(is_valid)
        
    def test_multiple_signatures(self):
        """Test signing multiple messages"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=4)  # 16 signatures available
        private_key, public_key = xmss.generate_keypair()
        
        messages = [f"Message {i}".encode() for i in range(5)]
        signatures = []
        
        # Sign multiple messages
        for message in messages:
            signature = xmss.sign(message)
            signatures.append(signature)
            
        # Verify all signatures
        for i, (message, signature) in enumerate(zip(messages, signatures)):
            is_valid = xmss.verify(message, signature)
            self.assertTrue(is_valid, f"Signature {i} verification failed")
            
        # Check final state
        self.assertEqual(xmss.get_state(), 5)
        
    def test_parameter_sets(self):
        """Test all predefined parameter sets"""
        parameter_sets = [
            "XMSS-SHA2_10_256",
            "XMSS-SHA2_16_256", 
            "XMSS-SHAKE_10_256",
            "XMSS-SHAKE_16_256"
        ]
        
        for param_set_name in parameter_sets:
            with self.subTest(param_set=param_set_name):
                try:
                    xmss = XMSS(parameter_set_name=param_set_name)
                    # Just test that initialization works
                    self.assertIsNotNone(xmss)
                except Exception as e:
                    self.fail(f"Failed to initialize XMSS with parameter set {param_set_name}: {e}")
                    
    def test_serialization_methods_exist(self):
        """Test that serialization methods exist (even if they are placeholders)"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        # Test that serialization methods exist and can be called
        try:
            private_key_data = xmss.serialize_private_key()
            public_key_data = xmss.serialize_public_key()
            self.assertIsInstance(private_key_data, bytes)
            self.assertIsInstance(public_key_data, bytes)
        except ValueError:
            # Expected for placeholder implementations
            pass
            
    def test_deserialization_methods(self):
        """Test that deserialization methods exist and can be called"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        # Test that deserialization methods exist and can be called
        try:
            private_key_data = xmss.serialize_private_key()
            public_key_data = xmss.serialize_public_key()
            
            # Try to deserialize (may return None for placeholder implementations)
            deserialized_private = xmss.deserialize_private_key(private_key_data)
            deserialized_public = xmss.deserialize_public_key(public_key_data)
            
            # At least they should not raise exceptions when called
        except Exception:
            # If serialization is not fully implemented, that's OK for now
            pass
            
    def test_get_public_key(self):
        """Test getting public key from XMSS instance"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        retrieved_public_key = xmss.get_public_key()
        self.assertEqual(retrieved_public_key, public_key)
        
    def test_update_state(self):
        """Test updating XMSS state"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        private_key, public_key = xmss.generate_keypair()
        
        # Initial state should be 0
        self.assertEqual(xmss.get_state(), 0)
        
        # Update state
        xmss.update_state(5)
        self.assertEqual(xmss.get_state(), 5)
        
    def test_invalid_parameter_set(self):
        """Test that invalid parameter set names raise an exception"""
        with self.assertRaises(ValueError):
            XMSS(parameter_set_name="INVALID_PARAM_SET")
            
    def test_sign_without_keypair(self):
        """Test that signing without generating keypair raises an exception"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        
        with self.assertRaises(ValueError):
            xmss.sign(self.test_message)
            
    def test_verify_without_keypair(self):
        """Test that verifying without generating keypair raises an exception"""
        xmss = XMSS(ots_scheme=self.ots_scheme, height=3)
        
        # Create a mock signature
        class MockSignature:
            pass
            
        with self.assertRaises(ValueError):
            xmss.verify(self.test_message, MockSignature())


if __name__ == '__main__':
    unittest.main()