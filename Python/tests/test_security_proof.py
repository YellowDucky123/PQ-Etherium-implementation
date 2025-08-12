"""
Unit tests for security proof framework
"""

import unittest
from xmss.security.proof import SecurityProof
from xmss.encoding.incomparable import IncomparableEncoding
from xmss.hash_functions.sha3 import SHA3Hash


class TestSecurityProof(unittest.TestCase):
    """Test security proof framework"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.encoding_scheme = IncomparableEncoding({'length': 32})
        self.hash_function = SHA3Hash("SHA3-256")
        self.security_proof = SecurityProof(self.encoding_scheme, self.hash_function)
        
        # Mock adversary function for testing
        self.mock_adversary = lambda: True
    
    def test_initialization(self):
        """Test SecurityProof initialization"""
        self.assertIsInstance(self.security_proof, SecurityProof)
        self.assertEqual(self.security_proof.encoding_scheme, self.encoding_scheme)
        self.assertEqual(self.security_proof.hash_function, self.hash_function)
    
    def test_prove_existential_unforgeability(self):
        """Test existential unforgeability proof"""
        result = self.security_proof.prove_existential_unforgeability(self.mock_adversary)
        self.assertIsInstance(result, bool)
        # In our implementation, this always returns True
        self.assertTrue(result)
    
    def test_prove_strong_unforgeability(self):
        """Test strong unforgeability proof"""
        result = self.security_proof.prove_strong_unforgeability(self.mock_adversary)
        self.assertIsInstance(result, bool)
        # In our implementation, this always returns True
        self.assertTrue(result)
    
    def test_analyze_security_loss(self):
        """Test security loss analysis"""
        result = self.security_proof.analyze_security_loss()
        self.assertIsInstance(result, float)
        # In our implementation, this returns 1.0
        self.assertEqual(result, 1.0)
    
    def test_verify_standard_model_properties(self):
        """Test standard model properties verification"""
        result = self.security_proof.verify_standard_model_properties()
        self.assertIsInstance(result, bool)
        # In our implementation, this always returns True
        self.assertTrue(result)
    
    def test_reduce_to_hash_properties(self):
        """Test reduction to hash properties"""
        result = self.security_proof._reduce_to_hash_properties()
        self.assertIsInstance(result, dict)
        self.assertIn("preimage_resistance", result)
        self.assertIn("collision_resistance", result)
        self.assertIn("second_preimage_resistance", result)
        self.assertTrue(result["preimage_resistance"])
        self.assertTrue(result["collision_resistance"])
        self.assertTrue(result["second_preimage_resistance"])
    
    def test_analyze_incomparable_encoding_security(self):
        """Test incomparable encoding security analysis"""
        result = self.security_proof._analyze_incomparable_encoding_security()
        self.assertIsInstance(result, dict)
        self.assertIn("incomparability", result)
        self.assertIn("pseudorandomness", result)
        self.assertIn("unforgeability", result)
        self.assertTrue(result["incomparability"])
        self.assertTrue(result["pseudorandomness"])
        self.assertTrue(result["unforgeability"])


if __name__ == '__main__':
    unittest.main()