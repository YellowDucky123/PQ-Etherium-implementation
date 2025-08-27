"""
Base Class for Encoding Schemes

This module defines the base interface for encoding schemes used in XMSS.
"""

from abc import ABC, abstractmethod
from typing import Any, List


class Encoding(ABC):
    """
    Abstract base class for encoding schemes.
    
    All encoding schemes used in XMSS should inherit from this class
    and implement its abstract methods.
    """
    
    @abstractmethod
    def encode(self, message: Any) -> List[Any]:
        """
        Encode a message into a list of values.
        
        Args:
            message: Message to encode
            
        Returns:
            List of encoded values
        """
        pass
    
    @abstractmethod
    def decode(self, encoded: List[Any]) -> Any:
        """
        Decode a list of values back to a message.
        
        Args:
            encoded: List of encoded values
            
        Returns:
            Decoded message
        """
        pass
    
    @abstractmethod
    def get_encoding_length(self) -> int:
        """
        Get the length of the encoding.
        
        Returns:
            Number of elements in the encoding
        """
        pass