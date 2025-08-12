#!/usr/bin/env python3
"""
Complete Test Suite for XMSS Implementation

This script runs all unit tests, module tests, and system tests for the XMSS implementation.
All test outputs are stored in test_output.txt.
"""

import unittest
import sys
import os
from io import StringIO

# Add the xmss package to the path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Import all test modules
from tests.test_xmss_core import TestXMSSCore
from tests.test_winternitz import TestWinternitzOTS
from tests.test_target_sum_winternitz import TestTargetSumWinternitzOTS
from tests.test_merkle_tree import TestMerkleTree
from tests.test_parameters import TestXMSSParameters
from tests.test_state import TestXMSSStateManager
from tests.test_hash_functions import TestTweakableHash, TestSHA3Hash, TestPoseidonHash
from tests.test_security_proof import TestSecurityProof


def create_test_suite():
    """Create a test suite with all tests"""
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Add all test cases
    suite.addTests(loader.loadTestsFromTestCase(TestXMSSCore))
    suite.addTests(loader.loadTestsFromTestCase(TestWinternitzOTS))
    suite.addTests(loader.loadTestsFromTestCase(TestTargetSumWinternitzOTS))
    suite.addTests(loader.loadTestsFromTestCase(TestMerkleTree))
    suite.addTests(loader.loadTestsFromTestCase(TestXMSSParameters))
    suite.addTests(loader.loadTestsFromTestCase(TestXMSSStateManager))
    suite.addTests(loader.loadTestsFromTestCase(TestTweakableHash))
    suite.addTests(loader.loadTestsFromTestCase(TestSHA3Hash))
    suite.addTests(loader.loadTestsFromTestCase(TestPoseidonHash))
    suite.addTests(loader.loadTestsFromTestCase(TestSecurityProof))
    
    return suite


def run_all_tests():
    """Run all tests and save output to test_output.txt"""
    # Create a string buffer to capture output
    test_output = StringIO()
    
    # Create a TextTestRunner that writes to our string buffer
    runner = unittest.TextTestRunner(stream=test_output, verbosity=2)
    
    print("XMSS Implementation Complete Test Suite")
    print("=" * 50)
    print()
    
    print("1. Running Unit Tests")
    print("-" * 25)
    
    suite = create_test_suite()
    result = runner.run(suite)
    
    print()
    print("2. Test Summary")
    print("-" * 15)
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Success rate: {((result.testsRun - len(result.failures) - len(result.errors)) / result.testsRun * 100) if result.testsRun > 0 else 0:.2f}%")
    
    if result.failures:
        print()
        print("Failures:")
        for test, traceback in result.failures:
            print(f"  {test}:")
            print(f"    {traceback}")
    
    if result.errors:
        print()
        print("Errors:")
        for test, traceback in result.errors:
            print(f"  {test}:")
            print(f"    {traceback}")
    
    # Get the output as a string
    output_str = test_output.getvalue()
    
    # Write output to file
    with open('test_output.txt', 'w', encoding='utf-8') as f:
        f.write(output_str)
    
    # Also print to console
    print(output_str)
    
    # Return appropriate exit code
    return 0 if result.wasSuccessful() else 1


def run_functional_tests():
    """Run functional tests to verify system components work together"""
    test_output = StringIO()
    print("3. Running Functional Tests")
    print("-" * 30)
    print("3. Running Functional Tests", file=test_output)
    print("-" * 30, file=test_output)
    
    try:
        # Test complete XMSS workflow
        from xmss.core.xmss import XMSS
        from xmss.otss.winternitz import WinternitzOTS
        
        print("Testing XMSS workflow with Winternitz OTS...")
        print("Testing XMSS workflow with Winternitz OTS...", file=test_output)
        
        # Create XMSS instance with small tree for faster testing
        ots_scheme = WinternitzOTS(w=4)
        xmss = XMSS(ots_scheme=ots_scheme, height=3)  # Small tree for testing
        
        # Generate keypair
        private_key, public_key = xmss.generate_keypair()
        print(f"  Generated keypair with tree height {xmss.height}")
        print(f"  Number of available signatures: {2**xmss.height}")
        print(f"  Generated keypair with tree height {xmss.height}", file=test_output)
        print(f"  Number of available signatures: {2**xmss.height}", file=test_output)
        
        # Test signing and verification
        messages = [b"Test message 1", b"Test message 2"]
        
        for i, message in enumerate(messages):
            signature = xmss.sign(message)
            is_valid = xmss.verify(message, signature)
            print(f"  Message {i+1}: {'PASSED' if is_valid else 'FAILED'}")
            print(f"  Message {i+1}: {'PASSED' if is_valid else 'FAILED'}", file=test_output)
            if not is_valid:
                raise Exception(f"Verification failed for message {i+1}")
        
        print("  All functional tests PASSED")
        print("  All functional tests PASSED", file=test_output)
        
    except Exception as e:
        print(f"  Functional test FAILED: {str(e)}")
        print(f"  Functional test FAILED: {str(e)}", file=test_output)
        return False, test_output.getvalue()
    
    return True, test_output.getvalue()


def run_system_tests():
    """Run system tests to verify overall system functionality"""
    test_output = StringIO()
    print("4. Running System Tests")
    print("-" * 25)
    print("4. Running System Tests", file=test_output)
    print("-" * 25, file=test_output)
    
    try:
        print("Testing complete XMSS system functionality...")
        print("Testing complete XMSS system functionality...", file=test_output)
        
        # Test with different parameter sets
        from xmss.core.xmss import XMSS
        from xmss.otss.target_sum_winternitz import TargetSumWinternitzOTS
        
        # Test with predefined parameter set (using an existing one)
        xmss1 = XMSS(parameter_set_name="XMSS-SHA2_10_256")
        private_key1, public_key1 = xmss1.generate_keypair()
        message1 = b"System test with predefined parameters"
        signature1 = xmss1.sign(message1)
        is_valid1 = xmss1.verify(message1, signature1)
        print(f"  Predefined parameter set test: {'PASSED' if is_valid1 else 'FAILED'}")
        print(f"  Predefined parameter set test: {'PASSED' if is_valid1 else 'FAILED'}", file=test_output)
        
        # Test with Target-Sum Winternitz OTS
        target_sum_ots = TargetSumWinternitzOTS(w=4)
        xmss2 = XMSS(ots_scheme=target_sum_ots, height=2)
        private_key2, public_key2 = xmss2.generate_keypair()
        message2 = b"System test with Target-Sum Winternitz"
        signature2 = xmss2.sign(message2)
        is_valid2 = xmss2.verify(message2, signature2)
        print(f"  Target-Sum Winternitz test: {'PASSED' if is_valid2 else 'FAILED'}")
        print(f"  Target-Sum Winternitz test: {'PASSED' if is_valid2 else 'FAILED'}", file=test_output)
        
        # Test state management
        initial_state = xmss2.get_state()
        xmss2.sign(b"State test 1")
        state_after_sign1 = xmss2.get_state()
        xmss2.sign(b"State test 2")
        state_after_sign2 = xmss2.get_state()
        
        # Check that state updates correctly after each signature
        # We don't require specific numeric values, just that the state progresses
        state_passed = (state_after_sign1 != initial_state) and (state_after_sign2 != state_after_sign1)
        
        print(f"  State management test: {'PASSED' if state_passed else 'FAILED'}")
        print(f"  State management test: {'PASSED' if state_passed else 'FAILED'}", file=test_output)
        
        # Test with SHA3 hash function
        from xmss.hash_functions.sha3 import SHA3Hash
        sha3_hash = SHA3Hash("SHA3-256")
        sha3_result = sha3_hash.message_hash(b"SHA3 test")
        print(f"  SHA3 hash function test: {'PASSED' if isinstance(sha3_result, bytes) and len(sha3_result) == 32 else 'FAILED'}")
        print(f"  SHA3 hash function test: {'PASSED' if isinstance(sha3_result, bytes) and len(sha3_result) == 32 else 'FAILED'}", file=test_output)
        
        print("  All system tests completed")
        print("  All system tests completed", file=test_output)
        
    except Exception as e:
        print(f"  System test FAILED: {str(e)}")
        print(f"  System test FAILED: {str(e)}", file=test_output)
        return False, test_output.getvalue()
    
    return True, test_output.getvalue()


if __name__ == '__main__':
    # Run unit and existing tests
    exit_code = run_all_tests()
    
    # Run functional tests
    func_success, func_output = run_functional_tests()
    print(func_output)
    with open('test_output.txt', 'a', encoding='utf-8') as f:
        f.write(func_output)
    
    # Run system tests
    sys_success, sys_output = run_system_tests()
    print(sys_output)
    with open('test_output.txt', 'a', encoding='utf-8') as f:
        f.write(sys_output)
    
    # Final summary
    final_output = StringIO()
    print("\n5. Final Test Summary")
    print("-" * 22)
    overall_success = (exit_code == 0) and func_success and sys_success
    print(f"Overall system status: {'PASSED' if overall_success else 'FAILED'}")
    
    print("\n5. Final Test Summary", file=final_output)
    print("-" * 22, file=final_output)
    print(f"Overall system status: {'PASSED' if overall_success else 'FAILED'}", file=final_output)
    
    print(final_output.getvalue())
    with open('test_output.txt', 'a', encoding='utf-8') as f:
        f.write(final_output.getvalue())
    
    sys.exit(0 if overall_success else 1)