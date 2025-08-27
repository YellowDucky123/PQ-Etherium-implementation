"""
Encoding Module

This module contains implementations of encoding schemes used in XMSS,
particularly incomparable encodings.
"""

from .base import Encoding
from .incomparable import IncomparableEncoding

__all__ = [
    "Encoding",
    "IncomparableEncoding"
]