"""
Unit tests for XMSS state management
"""

import unittest
import tempfile
import os
import json
from xmss.state import XMSSStateManager
from xmss.core.keygen import XMSSPrivateKey, XMSSPublicKey
from unittest.mock import Mock


class TestXMSSStateManager(unittest.TestCase):
    
    def setUp(self):
        """Set up test fixtures before each test method."""
        # Create a temporary file for state
        self.temp_file = tempfile.NamedTemporaryFile(delete=False)
        self.temp_file.close()
        self.state_manager = XMSSStateManager(self.temp_file.name)
        
    def tearDown(self):
        """Clean up after each test method."""
        if os.path.exists(self.temp_file.name):
            os.unlink(self.temp_file.name)
            
    def test_initialization(self):
        """Test XMSSStateManager initialization"""
        manager = XMSSStateManager("test_state.json")
        self.assertEqual(manager.state_file, "test_state.json")
        
    def test_save_state(self):
        """Test saving state"""
        # Create a mock private key
        private_key = Mock(spec=XMSSPrivateKey)
        private_key.index = 5
        private_key.height = 10
        
        # Save the state
        self.state_manager.save_state(private_key, "test_key")
        
        # Check that file was created and contains expected data
        self.assertTrue(os.path.exists(self.temp_file.name))
        
        with open(self.temp_file.name, 'r') as f:
            states = json.load(f)
            
        self.assertIn("test_key", states)
        self.assertEqual(states["test_key"]["index"], 5)
        self.assertEqual(states["test_key"]["height"], 10)
        self.assertEqual(states["test_key"]["identifier"], "test_key")
        
    def test_load_state(self):
        """Test loading state"""
        # Create initial state file
        initial_state = {
            "test_key": {
                "identifier": "test_key",
                "index": 3,
                "height": 10
            }
        }
        
        with open(self.temp_file.name, 'w') as f:
            json.dump(initial_state, f)
            
        # Load the state
        state_data = self.state_manager.load_state("test_key")
        
        self.assertIsNotNone(state_data)
        self.assertEqual(state_data["index"], 3)
        self.assertEqual(state_data["height"], 10)
        self.assertEqual(state_data["identifier"], "test_key")
        
    def test_load_nonexistent_state(self):
        """Test loading nonexistent state"""
        state_data = self.state_manager.load_state("nonexistent_key")
        self.assertIsNone(state_data)
        
    def test_load_without_state_file(self):
        """Test loading when state file doesn't exist"""
        # Create manager with nonexistent file
        manager = XMSSStateManager("nonexistent_file.json")
        state_data = manager.load_state("test_key")
        self.assertIsNone(state_data)
        
    def test_delete_state(self):
        """Test deleting state"""
        # Create initial state
        initial_state = {
            "test_key1": {"identifier": "test_key1", "index": 1, "height": 10},
            "test_key2": {"identifier": "test_key2", "index": 2, "height": 10}
        }
        
        with open(self.temp_file.name, 'w') as f:
            json.dump(initial_state, f)
            
        # Delete one key
        self.state_manager.delete_state("test_key1")
        
        # Check that only the other key remains
        with open(self.temp_file.name, 'r') as f:
            states = json.load(f)
            
        self.assertNotIn("test_key1", states)
        self.assertIn("test_key2", states)
        
    def test_delete_nonexistent_state(self):
        """Test deleting nonexistent state"""
        # Create initial state
        initial_state = {
            "test_key": {"identifier": "test_key", "index": 1, "height": 10}
        }
        
        with open(self.temp_file.name, 'w') as f:
            json.dump(initial_state, f)
            
        # Try to delete nonexistent key (should not raise exception)
        try:
            self.state_manager.delete_state("nonexistent_key")
            # If we get here, no exception was raised, which is what we want
            success = True
        except Exception:
            success = False
            
        self.assertTrue(success)
        
        # Original key should still be there
        with open(self.temp_file.name, 'r') as f:
            states = json.load(f)
            
        self.assertIn("test_key", states)
        
    def test_check_and_update_state(self):
        """Test checking and updating state"""
        # Create a mock private key
        private_key = Mock(spec=XMSSPrivateKey)
        private_key.index = 5
        private_key.height = 10
        
        # Check and update state
        result = self.state_manager.check_and_update_state(private_key, "test_key")
        
        # Should succeed (our implementation is a placeholder that always succeeds)
        self.assertTrue(result)
        
        # Check that state was saved
        state_data = self.state_manager.load_state("test_key")
        self.assertIsNotNone(state_data)


if __name__ == '__main__':
    unittest.main()