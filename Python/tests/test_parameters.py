"""
Unit tests for XMSS parameters
"""

import unittest
from xmss.parameters import XMSS_SHA2_10_256, XMSS_SHA2_16_256, XMSS_SHAKE_10_256, XMSS_SHAKE_16_256, get_parameters_by_name, XMSS_PARAMETER_SETS


class TestXMSSParameters(unittest.TestCase):
    
    def test_parameter_sets_exist(self):
        """Test that all predefined parameter sets exist"""
        param_sets = [
            XMSS_SHA2_10_256,
            XMSS_SHA2_16_256,
            XMSS_SHAKE_10_256,
            XMSS_SHAKE_16_256
        ]
        
        for param_set in param_sets:
            with self.subTest(param_set=param_set.name):
                self.assertIsNotNone(param_set)
                self.assertIsInstance(param_set.name, str)
                self.assertIsInstance(param_set.height, int)
                self.assertIsInstance(param_set.winternitz_w, int)
                self.assertIsInstance(param_set.hash_function, str)
                self.assertIsInstance(param_set.n, int)
                self.assertIsInstance(param_set.signature_bytes, int)
                
    def test_parameter_set_values(self):
        """Test specific values in parameter sets"""
        # Test XMSS-SHA2_10_256
        self.assertEqual(XMSS_SHA2_10_256.name, "XMSS-SHA2_10_256")
        self.assertEqual(XMSS_SHA2_10_256.height, 10)
        self.assertEqual(XMSS_SHA2_10_256.winternitz_w, 16)
        self.assertEqual(XMSS_SHA2_10_256.hash_function, "SHA2")
        self.assertEqual(XMSS_SHA2_10_256.n, 32)
        
        # Test XMSS-SHA2_16_256
        self.assertEqual(XMSS_SHA2_16_256.name, "XMSS-SHA2_16_256")
        self.assertEqual(XMSS_SHA2_16_256.height, 16)
        self.assertEqual(XMSS_SHA2_16_256.winternitz_w, 16)
        
        # Test XMSS-SHAKE_10_256
        self.assertEqual(XMSS_SHAKE_10_256.name, "XMSS-SHAKE_10_256")
        self.assertEqual(XMSS_SHAKE_10_256.height, 10)
        self.assertEqual(XMSS_SHAKE_10_256.winternitz_w, 16)
        
        # Test XMSS-SHAKE_16_256
        self.assertEqual(XMSS_SHAKE_16_256.name, "XMSS-SHAKE_16_256")
        self.assertEqual(XMSS_SHAKE_16_256.height, 16)
        self.assertEqual(XMSS_SHAKE_16_256.winternitz_w, 16)
        
    def test_get_parameters_by_name(self):
        """Test getting parameters by name"""
        param_set = get_parameters_by_name("XMSS-SHA2_10_256")
        self.assertEqual(param_set, XMSS_SHA2_10_256)
        
        param_set = get_parameters_by_name("XMSS-SHAKE_16_256")
        self.assertEqual(param_set, XMSS_SHAKE_16_256)
        
    def test_get_parameters_by_invalid_name(self):
        """Test that invalid parameter set names raise ValueError"""
        with self.assertRaises(ValueError):
            get_parameters_by_name("INVALID_PARAM_SET")
            
    def test_parameter_set_string_representation(self):
        """Test string representation of parameter sets"""
        self.assertEqual(str(XMSS_SHA2_10_256), "XMSS-SHA2_10_256")
        
    def test_all_parameter_sets_in_dict(self):
        """Test that all parameter sets are in the XMSS_PARAMETER_SETS dictionary"""
        expected_names = [
            "XMSS-SHA2_10_256",
            "XMSS-SHA2_16_256",
            "XMSS-SHAKE_10_256",
            "XMSS-SHAKE_16_256"
        ]
        
        for name in expected_names:
            self.assertIn(name, XMSS_PARAMETER_SETS)
            self.assertIsNotNone(XMSS_PARAMETER_SETS[name])
            
    def test_parameter_sets_consistency(self):
        """Test consistency between direct access and dictionary access"""
        self.assertEqual(XMSS_PARAMETER_SETS["XMSS-SHA2_10_256"], XMSS_SHA2_10_256)
        self.assertEqual(XMSS_PARAMETER_SETS["XMSS-SHA2_16_256"], XMSS_SHA2_16_256)
        self.assertEqual(XMSS_PARAMETER_SETS["XMSS-SHAKE_10_256"], XMSS_SHAKE_10_256)
        self.assertEqual(XMSS_PARAMETER_SETS["XMSS-SHAKE_16_256"], XMSS_SHAKE_16_256)
        
    def test_parameter_set_attributes(self):
        """Test that all parameter sets have expected attributes"""
        for name, param_set in XMSS_PARAMETER_SETS.items():
            with self.subTest(param_set=name):
                self.assertTrue(hasattr(param_set, 'name'))
                self.assertTrue(hasattr(param_set, 'height'))
                self.assertTrue(hasattr(param_set, 'winternitz_w'))
                self.assertTrue(hasattr(param_set, 'hash_function'))
                self.assertTrue(hasattr(param_set, 'n'))
                self.assertTrue(hasattr(param_set, 'signature_bytes'))
                
                # Check types
                self.assertIsInstance(param_set.name, str)
                self.assertIsInstance(param_set.height, int)
                self.assertIsInstance(param_set.winternitz_w, int)
                self.assertIsInstance(param_set.hash_function, str)
                self.assertIsInstance(param_set.n, int)
                self.assertIsInstance(param_set.signature_bytes, int)


if __name__ == '__main__':
    unittest.main()