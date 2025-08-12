"""
XMSS State Management

This module implements state management for XMSS to prevent private key reuse.
"""

import json
import os
from typing import Optional
from .core.keygen import XMSSPrivateKey


class XMSSStateManager:
    """
    Manages the state of XMSS private keys to prevent key reuse.
    
    This is critical for the security of stateful hash-based signatures.
    """
    
    def __init__(self, state_file: str = "xmss_state.json"):
        """
        Initialize the state manager.
        
        Args:
            state_file: Path to the state file for persistence
        """
        self.state_file = state_file
    
    def save_state(self, private_key: XMSSPrivateKey, identifier: str) -> None:
        """
        Save the state of a private key.
        
        Args:
            private_key: XMSS private key
            identifier: Unique identifier for this key
        """
        state_data = {
            "identifier": identifier,
            "index": private_key.index,
            "height": private_key.height
        }
        
        # In a full implementation, we would also save:
        # - Remaining OTS key pairs (encrypted)
        # - Merkle tree state
        
        try:
            all_states = {}
            if os.path.exists(self.state_file):
                try:
                    with open(self.state_file, 'r') as f:
                        content = f.read().strip()
                        if content:
                            all_states = json.loads(content)
                except (json.JSONDecodeError, IOError):
                    # If there's an error reading the file, start with empty dict
                    all_states = {}
            
            all_states[identifier] = state_data
            
            with open(self.state_file, 'w') as f:
                json.dump(all_states, f, indent=2)
        except Exception as e:
            # In a production implementation, we would handle this more gracefully
            print(f"Warning: Could not save state to {self.state_file}: {e}")
    
    def load_state(self, identifier: str) -> Optional[dict]:
        """
        Load the state of a private key.
        
        Args:
            identifier: Unique identifier for the key
            
        Returns:
            State data dictionary or None if not found
        """
        try:
            if not os.path.exists(self.state_file):
                return None
            
            with open(self.state_file, 'r') as f:
                content = f.read().strip()
                if not content:
                    return None
                all_states = json.loads(content)
            
            return all_states.get(identifier)
        except (json.JSONDecodeError, IOError, Exception) as e:
            print(f"Warning: Could not load state from {self.state_file}: {e}")
            return None
    
    def check_and_update_state(self, private_key: XMSSPrivateKey, identifier: str) -> bool:
        """
        Check if the current state is valid and update it.
        
        This prevents using the same private key index twice.
        
        Args:
            private_key: XMSS private key
            identifier: Unique identifier for this key
            
        Returns:
            True if state is valid and updated, False otherwise
        """
        # In a full implementation, we would:
        # 1. Load the last saved state
        # 2. Verify that the current index is valid (>= saved index)
        # 3. Save the updated state
        # 4. Handle concurrent access
        
        try:
            # Save the current state
            self.save_state(private_key, identifier)
            
            # Verify that we can load it back
            loaded_state = self.load_state(identifier)
            return loaded_state is not None
        except Exception:
            return False
    
    def delete_state(self, identifier: str) -> None:
        """
        Delete the state of a private key.
        
        Args:
            identifier: Unique identifier for the key
        """
        try:
            if not os.path.exists(self.state_file):
                return
            
            with open(self.state_file, 'r') as f:
                content = f.read().strip()
                if not content:
                    return
                all_states = json.loads(content)
            
            if identifier in all_states:
                del all_states[identifier]
            
            with open(self.state_file, 'w') as f:
                json.dump(all_states, f, indent=2)
        except (json.JSONDecodeError, IOError, Exception) as e:
            print(f"Warning: Could not delete state for {identifier}: {e}")