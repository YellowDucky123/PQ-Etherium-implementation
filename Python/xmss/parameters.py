"""
XMSS Parameters according to RFC 8391

This module defines the standard XMSS parameters as specified in RFC 8391.
"""

from dataclasses import dataclass
from typing import Tuple
from .hash_functions.sha3 import SHA3Hash


@dataclass
class XMSSParameters:
    """XMSS parameter set as defined in RFC 8391"""
    name: str
    height: int
    winternitz_w: int
    hash_function: str
    n: int  # security parameter (bytes)
    signature_bytes: int
    
    def __str__(self) -> str:
        return self.name


# Predefined XMSS parameter sets from RFC 8391
XMSS_SHA2_10_256 = XMSSParameters(
    name="XMSS-SHA2_10_256",
    height=10,
    winternitz_w=16,
    hash_function="SHA2",
    n=32,
    signature_bytes=2500
)

XMSS_SHA2_16_256 = XMSSParameters(
    name="XMSS-SHA2_16_256",
    height=16,
    winternitz_w=16,
    hash_function="SHA2",
    n=32,
    signature_bytes=3988
)

XMSS_SHAKE_10_256 = XMSSParameters(
    name="XMSS-SHAKE_10_256",
    height=10,
    winternitz_w=16,
    hash_function="SHAKE",
    n=32,
    signature_bytes=2500
)

XMSS_SHAKE_16_256 = XMSSParameters(
    name="XMSS-SHAKE_16_256",
    height=16,
    winternitz_w=16,
    hash_function="SHAKE",
    n=32,
    signature_bytes=3988
)

# All supported parameter sets
XMSS_PARAMETER_SETS = {
    "XMSS-SHA2_10_256": XMSS_SHA2_10_256,
    "XMSS-SHA2_16_256": XMSS_SHA2_16_256,
    "XMSS-SHAKE_10_256": XMSS_SHAKE_10_256,
    "XMSS-SHAKE_16_256": XMSS_SHAKE_16_256
}


def get_parameters_by_name(name: str) -> XMSSParameters:
    """
    Get XMSS parameters by name.
    
    Args:
        name: Name of the parameter set
        
    Returns:
        XMSSParameters object
        
    Raises:
        ValueError: If parameter set name is not found
    """
    if name not in XMSS_PARAMETER_SETS:
        raise ValueError(f"Unknown XMSS parameter set: {name}")
    return XMSS_PARAMETER_SETS[name]