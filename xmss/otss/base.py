"""
Base Class for One-Time Signature Schemes

This module defines the base interface for one-time signature schemes
used in XMSS constructions.
"""

from abc import ABC, abstractmethod
from typing import Tuple, Any


class OTS(ABC):
    """
    Abstract base class for One-Time Signature schemes.
    
    All OTS implementations used in XMSS should inherit from this class
    and implement its abstract methods.
    """
    
    @abstractmethod
    def generate_keypair(self) -> Tuple[Any, Any]:
        """
        Generate a one-time signature key pair.
        
        Returns:
            Tuple of (private_key, public_key)
        """
        pass
    
    @abstractmethod
    def sign(self, message: bytes, private_key: Any) -> Any:
        """
        Sign a message with a one-time private key.
        
        Args:
            message: Message to sign
            private_key: One-time private key
            
        Returns:
            One-time signature
        """
        pass
    
    @abstractmethod
    def verify(self, message: bytes, signature: Any, public_key: Any) -> bool:
        """
        Verify a one-time signature.
        
        Args:
            message: Message that was signed
            signature: One-time signature to verify
            public_key: One-time public key for verification
            
        Returns:
            True if signature is valid, False otherwise
        """
        pass
    
    @abstractmethod
    def get_public_key_from_signature(self, message: bytes, signature: Any) -> Any:
        """
        Compute the public key from a signature and message.
        
        This is needed for some OTS schemes during XMSS verification.
        
        Args:
            message: Message that was signed
            signature: One-time signature
            
        Returns:
            Computed public key
        """
        pass