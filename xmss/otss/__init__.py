"""
One-Time Signature Schemes (OTSS) Module

This module contains implementations of various one-time signature schemes
used in the XMSS construction, including Winternitz OTS and variants.
"""

from .base import OTS
from .winternitz import WinternitzOTS
from .target_sum_winternitz import TargetSumWinternitzOTS

__all__ = [
    "OTS",
    "WinternitzOTS",
    "TargetSumWinternitzOTS"
]