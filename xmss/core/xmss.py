"""
XMSS (eXtended Merkle Signature Scheme) Implementation

This module implements the main XMSS signature scheme as described in the paper
'Hash-Based Post-Quantum Signatures: A Study of XMSS Variants'.
"""

from typing import Tuple, Any
from ..otss.base import OTS
from ..merkle.tree import MerkleTree
from ..encoding.incomparable import IncomparableEncoding
from .keygen import generate_xmss_keypair, XMSSPrivateKey, XMSSPublicKey
from .signing import sign_message, XMSSSignature
from .verification import verify_signature


class XMSS:
    """
    XMSS (eXtended Merkle Signature Scheme) main class.
    
    Implements the generalized XMSS framework SIG[IncEnc,Th,K] based on 
    incomparable encodings as described in the paper.
    """
    
    def __init__(self, ots_scheme: OTS, height: int, encoding: IncomparableEncoding = None):
        """
        Initialize the XMSS signature scheme.
        
        Args:
            ots_scheme: One-time signature scheme to use (e.g., Winternitz OTS)
            height: Height of the Merkle tree
            encoding: Incomparable encoding scheme to use
        """
        self.ots_scheme = ots_scheme
        self.height = height
        self.encoding = encoding
        self.private_key = None
        self.public_key = None
    
    def generate_keypair(self) -> Tuple[XMSSPrivateKey, XMSSPublicKey]:
        """
        Generate a key pair for the XMSS signature scheme.
        
        Returns:
            Tuple of (private_key, public_key) for the XMSS scheme
        """
        self.private_key, self.public_key = generate_xmss_keypair(self.ots_scheme, self.height)
        return self.private_key, self.public_key
    
    def sign(self, message: bytes, private_key: XMSSPrivateKey = None) -> XMSSSignature:
        """
        Sign a message using the XMSS signature scheme.
        
        Args:
            message: Message to sign
            private_key: Private key to use for signing (optional if keypair was generated)
            
        Returns:
            Signature for the message
        """
        key_to_use = private_key or self.private_key
        if key_to_use is None:
            raise ValueError("No private key available for signing")
            
        return sign_message(message, key_to_use)
    
    def verify(self, message: bytes, signature: XMSSSignature, public_key: XMSSPublicKey = None) -> bool:
        """
        Verify a signature using the XMSS signature scheme.
        
        Args:
            message: Message that was signed
            signature: Signature to verify
            public_key: Public key to use for verification (optional if keypair was generated)
            
        Returns:
            True if signature is valid, False otherwise
        """
        key_to_use = public_key or self.public_key
        if key_to_use is None:
            raise ValueError("No public key available for verification")
            
        return verify_signature(message, signature, key_to_use, self.ots_scheme)
    
    def get_public_key(self) -> XMSSPublicKey:
        """
        Get the public key of the XMSS scheme.
        
        Returns:
            Public key of the XMSS scheme
        """
        return self.public_key
    
    def get_state(self) -> int:
        """
        Get the current state of the XMSS signer.
        
        Returns:
            Current index/state information
        """
        if self.private_key:
            return self.private_key.index
        return 0
    
    def update_state(self, new_index: int) -> None:
        """
        Update the state of the XMSS signer.
        
        Args:
            new_index: New index value
        """
        if self.private_key:
            self.private_key.index = new_index