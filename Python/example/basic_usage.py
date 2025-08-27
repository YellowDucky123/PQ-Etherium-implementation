"""
Basic usage example of the XMSS implementation
"""

from xmss.core.xmss import XMSS
from xmss.otss.winternitz import WinternitzOTS


def main():
    print("XMSS Signature Scheme Example")
    print("=" * 40)
    
    # Create a Winternitz OTS scheme
    ots_scheme = WinternitzOTS(w=4)  # Using w=4 for a smaller example
    
    # Create XMSS scheme with a small tree height for demo
    xmss = XMSS(ots_scheme=ots_scheme, height=3)  # 2^3 = 8 possible signatures
    
    # Generate keypair
    print("Generating XMSS keypair...")
    private_key, public_key = xmss.generate_keypair()
    print(f"Generated keypair with Merkle tree height {xmss.height}")
    print(f"Available signatures: {2**xmss.height}")
    print(f"Current state (used signatures): {xmss.get_state()}")
    print()
    
    # Sign a message
    message = b"Hello, XMSS!"
    print(f"Signing message: {message}")
    
    try:
        signature = xmss.sign(message)
        print(f"Message signed successfully")
        print(f"Signature index: {signature.index}")
        print(f"State after signing: {xmss.get_state()}")
        print()
        
        # Verify the signature
        is_valid = xmss.verify(message, signature)
        print(f"Signature verification: {'PASSED' if is_valid else 'FAILED'}")
        
        # Try verifying with a wrong message
        wrong_message = b"Wrong message"
        is_valid_wrong = xmss.verify(wrong_message, signature)
        print(f"Verification with wrong message: {'PASSED' if is_valid_wrong else 'FAILED'}")
        print()
        
        # Sign a few more messages to demonstrate statefulness
        print("Signing additional messages:")
        for i in range(3):
            additional_message = f"Message {i+2}".encode()
            try:
                sig = xmss.sign(additional_message)
                valid = xmss.verify(additional_message, sig)
                print(f"  Message {i+2}: {'PASSED' if valid else 'FAILED'} (index: {sig.index})")
            except Exception as e:
                print(f"  Message {i+2}: ERROR - {e}")
        
        print(f"Final state: {xmss.get_state()}")
        
    except Exception as e:
        print(f"Error during signing: {e}")


if __name__ == "__main__":
    main()
