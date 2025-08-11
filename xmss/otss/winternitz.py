"""
Classical Winternitz One-Time Signature Implementation

This module implements the classical Winternitz one-time signature scheme
with checksum as described in the XMSS paper.
"""

from typing import Tuple, List
from .base import OTS
import hashlib
import os


class WinternitzOTS(OTS):
    """
    Classical Winternitz One-Time Signature scheme.
    
    Uses checksum to ensure security and includes the traditional approach
    with verification requiring more hash operations.
    """
    
    def __init__(self, w: int = 4):
        """
        Initialize the Winternitz OTS.
        
        Args:
            w: Winternitz parameter (typically 4 or 16)
        """
        if w not in [1, 2, 4, 8, 16]:
            raise ValueError("Winternitz parameter w must be 1, 2, 4, 8, or 16")
            
        self.w = w  # Winternitz parameter
        self.hash_function = hashlib.sha256
        # Precompute parameters for 256-bit message
        self.msg_bitlen = 256
        self.num_msg_blocks = self.msg_bitlen // self.w
        self.checksum_bits = self._get_checksum_bits()
        self.num_checksum_blocks = (self.checksum_bits + self.w - 1) // w
        self.num_blocks = self.num_msg_blocks + self.num_checksum_blocks
        self.max_chain_len = (1 << self.w) - 1  # 2^w - 1
    
    def generate_keypair(self) -> Tuple[List[bytes], List[bytes]]:
        """
        Generate a Winternitz one-time signature key pair.
        
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
    
    def sign(self, message: bytes, private_key: List[bytes]) -> List[bytes]:
        """
        Sign a message using the Winternitz OTS.
        
        Args:
            message: Message to sign
            private_key: Winternitz private key
            
        Returns:
            Winternitz signature
        """
        # Encode message with checksum
        encoded = self._encode_message_with_checksum(message)
        
        # Apply chain hashing for each block
        signature = []
        for i, chain_length in enumerate(encoded):
            if i >= len(private_key):
                break
            sig_element = self._chain_hash(private_key[i], chain_length, i)
            signature.append(sig_element)
        
        return signature
    
    def verify(self, message: bytes, signature: List[bytes], public_key: List[bytes]) -> bool:
        """
        Verify a Winternitz signature.
        
        Args:
            message: Message that was signed
            signature: Winternitz signature to verify
            public_key: Winternitz public key for verification
            
        Returns:
            True if signature is valid, False otherwise
        """
        try:
            # Check lengths
            if len(signature) != len(public_key):
                return False
                
            # Encode message with checksum
            encoded = self._encode_message_with_checksum(message)
            
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
    
    def get_public_key_from_signature(self, message: bytes, signature: List[bytes]) -> List[bytes]:
        """
        Compute the public key from a signature and message.
        
        Args:
            message: Message that was signed
            signature: Winternitz signature
            
        Returns:
            Computed public key
        """
        # Encode message with checksum
        encoded = self._encode_message_with_checksum(message)
        
        # Reconstruct public key elements from signature
        reconstructed_pubkey = []
        for i, sig_element in enumerate(signature):
            remaining_chain_len = self.max_chain_len - encoded[i]
            reconstructed_element = self._chain_hash(sig_element, remaining_chain_len, i)
            reconstructed_pubkey.append(reconstructed_element)
        
        return reconstructed_pubkey
    
    def _encode_message_with_checksum(self, message: bytes) -> List[int]:
        """
        Encode a message into a list of integers with checksum.
        
        Args:
            message: Message to encode
            
        Returns:
            List of integers representing the encoded message with checksum
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
        for i in range(self.num_msg_blocks):
            start = i * self.w
            end = start + self.w
            block_bits = msg_bits[start:end]
            blocks.append(int(block_bits, 2))
        
        # Calculate and append checksum
        checksum = self._compute_checksum(blocks)
        blocks.extend(checksum)
        
        # Ensure we have the right number of blocks
        while len(blocks) < self.num_blocks:
            blocks.append(0)
        
        return blocks[:self.num_blocks]
    
    def _compute_checksum(self, message_blocks: List[int]) -> List[int]:
        """
        Compute the checksum for the message blocks.
        
        Args:
            message_blocks: Message blocks without checksum
            
        Returns:
            Checksum blocks
        """
        # Sum all the blocks
        total = sum((1 << self.w) - 1 - block for block in message_blocks)
        
        # Split total into w-bit blocks
        checksum_blocks = []
        temp_total = total
        for _ in range(self.num_checksum_blocks):
            checksum_blocks.append(temp_total & ((1 << self.w) - 1))
            temp_total >>= self.w
        
        # Reverse to match specification
        checksum_blocks.reverse()
        return checksum_blocks
    
    def _get_checksum_bits(self) -> int:
        """
        Calculate the number of bits needed for the checksum.
        
        Returns:
            Number of checksum bits
        """
        # For a message of 256 bits divided into w-bit blocks
        max_checksum = self.num_msg_blocks * ((1 << self.w) - 1)
        # Number of bits needed to represent max_checksum
        return max_checksum.bit_length()
    
    def _chain_hash(self, start: bytes, chain_length: int, index: int) -> bytes:
        """
        Apply chain hashing to a starting value.
        
        Args:
            start: Starting value
            chain_length: Number of hash applications
            index: Index for the chain (used for domain separation)
            
        Returns:
            Result after chain hashing
        """
        result = start
        for _ in range(chain_length):
            # Simple hash without complex tweaks to ensure chain property
            result = self.hash_function(result + bytes([index])).digest()
        return result