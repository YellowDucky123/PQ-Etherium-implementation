"""
Target-Sum Winternitz One-Time Signature Implementation

This module implements the target-sum Winternitz one-time signature scheme
without checksum as described in the XMSS paper, which reduces verification
hash operations.
"""

from typing import Tuple, Any, List
from .base import OTS
from ..hash_functions.tweakable import TweakableHash
import hashlib
import os


class TargetSumWinternitzOTS(OTS):
    """
    Target-Sum Winternitz One-Time Signature scheme.
    
    Eliminates the checksum to reduce verification hash operations,
    using a target sum T = ⌈δv(2^w - 1)/2⌉ to ensure encoding properties.
    """
    
    def __init__(self, hash_function: TweakableHash = None, w: int = 16):
        """
        Initialize the Target-Sum Winternitz OTS.
        
        Args:
            hash_function: Tweakable hash function to use
            w: Winternitz parameter (typically 4 or 16)
        """
        if w not in [1, 2, 4, 8, 16]:
            raise ValueError("Winternitz parameter w must be 1, 2, 4, 8, or 16")
            
        self.w = w
        self.hash_function = hash_function or hashlib.sha256
        self.msg_bitlen = 256
        self.num_msg_blocks = self.msg_bitlen // self.w
        # For target-sum encoding, we use the same number of blocks
        self.num_blocks = self.num_msg_blocks
        self.max_chain_len = (1 << self.w) - 1  # 2^w - 1
        self.target_sum = self._calculate_target_sum(self.num_msg_blocks)
    
    def generate_keypair(self) -> Tuple[Any, Any]:
        """
        Generate a Target-Sum Winternitz one-time signature key pair.
        
        Returns:
            Tuple of (private_key, public_key)
        """
        # Generate private key (random values for each chain)
        private_key = []
        for i in range(self.num_blocks):
            # Use cryptographically secure random
            priv_element = os.urandom(32)
            private_key.append(priv_element)
        
        # Generate public key by applying chain hash to each private element
        public_key = []
        for i, priv_element in enumerate(private_key):
            pub_element = self._chain_hash(priv_element, self.max_chain_len, i)
            public_key.append(pub_element)
        
        return private_key, public_key
    
    def sign(self, message: bytes, private_key: Any) -> Any:
        """
        Sign a message using the Target-Sum Winternitz OTS.
        
        Args:
            message: Message to sign
            private_key: Target-Sum Winternitz private key
            
        Returns:
            Target-Sum Winternitz signature
        """
        # Encode message with target sum
        encoded = self._encode_message_with_target_sum(message)
        
        # Apply chain hashing for each block
        signature = []
        for i, chain_length in enumerate(encoded):
            if i >= len(private_key):
                break
            sig_element = self._chain_hash(private_key[i], chain_length, i)
            signature.append(sig_element)
        
        return signature
    
    def verify(self, message: bytes, signature: Any, public_key: Any) -> bool:
        """
        Verify a Target-Sum Winternitz signature.
        
        Args:
            message: Message that was signed
            signature: Target-Sum Winternitz signature to verify
            public_key: Target-Sum Winternitz public key for verification
            
        Returns:
            True if signature is valid, False otherwise
        """
        try:
            # Check lengths
            if len(signature) != len(public_key):
                return False
                
            # Encode message with target sum
            encoded = self._encode_message_with_target_sum(message)
            
            # Reconstruct public key elements from signature
            reconstructed_pubkey = []
            for i, sig_element in enumerate(signature):
                if i >= len(encoded):  # Safety check
                    return False
                remaining_chain_len = self.max_chain_len - encoded[i]
                reconstructed_element = self._chain_hash(sig_element, remaining_chain_len, i)
                reconstructed_pubkey.append(reconstructed_element)
            
            # Compare with provided public key
            return reconstructed_pubkey == public_key
        except Exception:
            # Any exception during verification means the signature is invalid
            return False
    
    def get_public_key_from_signature(self, message: bytes, signature: Any) -> Any:
        """
        Compute the public key from a signature and message.
        
        Args:
            message: Message that was signed
            signature: Target-Sum Winternitz signature
            
        Returns:
            Computed public key
        """
        # Encode message with target sum
        encoded = self._encode_message_with_target_sum(message)
        
        # Reconstruct public key elements from signature
        reconstructed_pubkey = []
        for i, sig_element in enumerate(signature):
            if i >= len(encoded):  # Safety check
                break
            remaining_chain_len = self.max_chain_len - encoded[i]
            reconstructed_element = self._chain_hash(sig_element, remaining_chain_len, i)
            reconstructed_pubkey.append(reconstructed_element)
        
        return reconstructed_pubkey
    
    def _encode_message_with_target_sum(self, message: bytes) -> List[int]:
        """
        Encode a message into a list of integers with target sum.
        
        Args:
            message: Message to encode
            
        Returns:
            List of integers representing the encoded message with target sum property
        """
        # Convert message to bits and pad to 256 bits
        msg_bits = ''.join(format(byte, '08b') for byte in message)
        # Pad or truncate to 256 bits
        if len(msg_bits) < self.msg_bitlen:
            msg_bits += '0' * (self.msg_bitlen - len(msg_bits))
        else:
            msg_bits = msg_bits[:self.msg_bitlen]
        
        # Split into w-bit blocks
        blocks = []
        for i in range(self.num_msg_blocks - 1):  # One less block for target sum
            start = i * self.w
            end = start + self.w
            block_bits = msg_bits[start:end]
            blocks.append(int(block_bits, 2))
        
        # Calculate the last block to meet target sum
        current_sum = sum(blocks)
        last_block = self.target_sum - current_sum
        
        # Ensure the last block is within valid range [0, max_chain_len]
        if last_block < 0:
            # Adjust previous blocks to make last block valid
            adjustment = -last_block
            # Distribute adjustment among previous blocks from right to left
            for i in range(len(blocks) - 1, -1, -1):
                while adjustment > 0 and blocks[i] > 0:
                    blocks[i] -= 1
                    adjustment -= 1
                    current_sum -= 1
                    if adjustment == 0:
                        break
                if adjustment == 0:
                    break
            last_block = self.target_sum - current_sum
        elif last_block > self.max_chain_len:
            # Adjust previous blocks to make last block valid
            adjustment = last_block - self.max_chain_len
            # Distribute adjustment among previous blocks from left to right
            for i in range(len(blocks)):
                while adjustment > 0 and blocks[i] < self.max_chain_len:
                    blocks[i] += 1
                    adjustment -= 1
                    current_sum += 1
                    if adjustment == 0:
                        break
                if adjustment == 0:
                    break
            last_block = self.target_sum - current_sum
        
        # Final check and clamp
        last_block = max(0, min(self.max_chain_len, last_block))
        blocks.append(last_block)
        
        # Make sure the sum is exactly target_sum
        final_sum = sum(blocks)
        if final_sum != self.target_sum:
            diff = self.target_sum - final_sum
            # Adjust the last element if possible
            if 0 <= blocks[-1] + diff <= self.max_chain_len:
                blocks[-1] += diff
            else:
                # Find another element to adjust
                for i in range(len(blocks) - 2, -1, -1):
                    if diff > 0 and blocks[i] <= self.max_chain_len - diff:
                        blocks[i] += diff
                        break
                    elif diff < 0 and blocks[i] >= -diff:
                        blocks[i] += diff
                        break
        
        return blocks
    
    def _calculate_target_sum(self, encoded_parts: int) -> int:
        """
        Calculate the target sum T = ⌈δv(2^w - 1)/2⌉.
        
        Args:
            encoded_parts: Number of parts in encoded message
            
        Returns:
            Target sum value
        """
        # T = ⌈(number_of_parts * (2^w - 1)) / 2⌉
        # For w=4, max_value = 15, so target_sum = ⌈(64 * 15) / 2⌉ = ⌈480⌉ = 480
        max_value = (1 << self.w) - 1  # 2^w - 1
        target_sum = (encoded_parts * max_value) // 2
        # Use ceiling division
        if (encoded_parts * max_value) % 2 != 0:
            target_sum += 1
        return target_sum
    
    def _adjust_encoding_to_target_sum(self, encoded_message: List[int], target_sum: int) -> List[int]:
        """
        Adjust encoding to meet target sum requirement.
        
        Args:
            encoded_message: Initial encoded message
            target_sum: Target sum to achieve
            
        Returns:
            Adjusted encoded message meeting target sum
        """
        # This is handled in _encode_message_with_target_sum
        return encoded_message
    
    def _chain_hash(self, start: Any, chain_length: int, index: int) -> Any:
        """
        Apply chain hashing to a starting value.
        
        Args:
            start: Starting value
            chain_length: Number of hash applications
            index: Index for tweakable hash function
            
        Returns:
            Result after chain hashing
        """
        result = start
        for i in range(chain_length):
            # Use tweakable hash if available
            if hasattr(self.hash_function, 'hash_with_tweak'):
                tweak = (index * self.max_chain_len + i).to_bytes(4, 'big')
                result = self.hash_function.hash_with_tweak(result, tweak)
            else:
                # Fallback to standard hash with index information (matching Winternitz)
                hasher = self.hash_function()
                hasher.update(result + bytes([index]))
                result = hasher.digest()
        return result