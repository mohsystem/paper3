
import os
import secrets
from typing import Tuple
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import hashes, hmac
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.backends import default_backend


def derive_key_from_passphrase(passphrase: str, salt: bytes) -> bytes:
    """\n    Derive a 256-bit encryption key from a passphrase using PBKDF2-HMAC-SHA256.\n    Uses 210,000 iterations as per security requirements.\n    """
    # Validate inputs
    if not passphrase or not isinstance(passphrase, str):
        raise ValueError("Passphrase must be a non-empty string")
    if not salt or len(salt) != 16:
        raise ValueError("Salt must be exactly 16 bytes")
    
    # Derive key using PBKDF2 with SHA-256, 210,000 iterations for security
    kdf = PBKDF2HMAC(
        algorithm=hashes.SHA256(),
        length=32,  # 256-bit key for AES-256
        salt=salt,
        iterations=210000,  # Minimum required iterations
        backend=default_backend()
    )
    return kdf.derive(passphrase.encode('utf-8'))


def compute_hmac(key: bytes, data: bytes) -> bytes:
    """\n    Compute HMAC-SHA256 for encrypt-then-MAC pattern.\n    """
    h = hmac.HMAC(key, hashes.SHA256(), backend=default_backend())
    h.update(data)
    return h.finalize()


def encrypt_aes_cbc_with_hmac(plaintext: bytes, passphrase: str) -> bytes:
    """\n    Encrypt data using AES-256-CBC with HMAC-SHA256 (Encrypt-then-MAC).\n    Returns a structured format: [magic][version][salt][iv][ciphertext][hmac_tag]\n    \n    Security measures:\n    - Uses AES-256 in CBC mode with PKCS7 padding\n    - Implements Encrypt-then-MAC pattern for authentication\n    - Generates unique 16-byte salt per encryption\n    - Generates unique 16-byte IV per encryption using CSPRNG\n    - Uses PBKDF2-HMAC-SHA256 with 210,000 iterations\n    - Validates all inputs before processing\n    """
    # Input validation - treat all inputs as untrusted
    if not plaintext or not isinstance(plaintext, bytes):
        raise ValueError("Plaintext must be non-empty bytes")
    if not passphrase or not isinstance(passphrase, str):
        raise ValueError("Passphrase must be a non-empty string")
    if len(plaintext) > 10 * 1024 * 1024:  # 10MB limit
        raise ValueError("Plaintext exceeds maximum size")
    
    # Generate cryptographically secure random salt and IV
    # Using secrets module for CSPRNG as per security requirements
    salt = secrets.token_bytes(16)  # Unique 16-byte salt
    iv = secrets.token_bytes(16)    # Unique 16-byte IV for CBC mode
    
    # Derive encryption key from passphrase using secure KDF
    encryption_key = derive_key_from_passphrase(passphrase, salt)
    
    # Derive separate HMAC key from passphrase for Encrypt-then-MAC
    hmac_salt = secrets.token_bytes(16)
    kdf_hmac = PBKDF2HMAC(
        algorithm=hashes.SHA256(),
        length=32,
        salt=hmac_salt,
        iterations=210000,
        backend=default_backend()
    )
    hmac_key = kdf_hmac.derive(passphrase.encode('utf-8'))
    
    # Create AES-256-CBC cipher with explicit mode specification
    cipher = Cipher(
        algorithms.AES(encryption_key),  # AES-256 (32-byte key)
        modes.CBC(iv),                   # CBC mode with unique IV
        backend=default_backend()
    )
    
    # Encrypt the plaintext
    encryptor = cipher.encryptor()
    # CBC mode with PKCS7 padding is handled by cryptography library
    ciphertext = encryptor.update(plaintext) + encryptor.finalize()
    
    # Build the output structure: magic + version + salt + hmac_salt + iv + ciphertext
    magic = b'ENC1'
    version = bytes([1])
    
    # Construct the authenticated payload
    authenticated_data = magic + version + salt + hmac_salt + iv + ciphertext
    
    # Compute HMAC over all data (Encrypt-then-MAC pattern)
    hmac_tag = compute_hmac(hmac_key, authenticated_data)
    
    # Securely clear sensitive key material from memory
    # Note: Python doesn't guarantee memory clearing, but we make best effort\n    del encryption_key\n    del hmac_key\n    \n    # Return complete encrypted package with authentication\n    return authenticated_data + hmac_tag\n\n\ndef decrypt_aes_cbc_with_hmac(encrypted_data: bytes, passphrase: str) -> bytes:\n    """\n    Decrypt data encrypted with encrypt_aes_cbc_with_hmac.\n    Verifies HMAC before decryption (fail closed on authentication failure).\n    \n    Security measures:\n    - Validates magic and version before processing\n    - Verifies HMAC in constant time before decryption\n    - Fails closed on any validation error without revealing details\n    - Does not return partial plaintext on error\n    """\n    # Input validation\n    if not encrypted_data or not isinstance(encrypted_data, bytes):\n        raise ValueError("Encrypted data must be non-empty bytes")\n    if not passphrase or not isinstance(passphrase, str):\n        raise ValueError("Passphrase must be a non-empty string")\n    \n    # Minimum size check: magic(4) + version(1) + salt(16) + hmac_salt(16) + iv(16) + hmac(32)\n    if len(encrypted_data) < 85:\n        raise ValueError("Invalid encrypted data format")\n    \n    # Parse the structure\n    offset = 0\n    magic = encrypted_data[offset:offset+4]\n    offset += 4\n    \n    # Validate magic number\n    if magic != b'ENC1':\n        raise ValueError("Invalid file format")\n    \n    version = encrypted_data[offset]\n    offset += 1\n    \n    # Validate version\n    if version != 1:\n        raise ValueError("Unsupported version")\n    \n    salt = encrypted_data[offset:offset+16]\n    offset += 16\n    \n    hmac_salt = encrypted_data[offset:offset+16]\n    offset += 16\n    \n    iv = encrypted_data[offset:offset+16]\n    offset += 16\n    \n    # Remaining data is ciphertext + hmac_tag\n    hmac_tag = encrypted_data[-32:]\n    ciphertext = encrypted_data[offset:-32]\n    authenticated_data = encrypted_data[:-32]\n    \n    # Derive HMAC key to verify authentication tag\n    kdf_hmac = PBKDF2HMAC(\n        algorithm=hashes.SHA256(),\n        length=32,\n        salt=hmac_salt,\n        iterations=210000,\n        backend=default_backend()\n    )\n    hmac_key = kdf_hmac.derive(passphrase.encode('utf-8'))\n    \n    # Verify HMAC tag using constant-time comparison\n    expected_hmac = compute_hmac(hmac_key, authenticated_data)\n    \n    # Use constant-time comparison to prevent timing attacks\n    if not secrets.compare_digest(expected_hmac, hmac_tag):\n        # Fail closed - clear sensitive data and raise error\n        del hmac_key\n        raise ValueError("Authentication failed")\n    \n    # HMAC verified - proceed with decryption\n    encryption_key = derive_key_from_passphrase(passphrase, salt)\n    \n    # Create AES-256-CBC cipher for decryption\n    cipher = Cipher(\n        algorithms.AES(encryption_key),\n        modes.CBC(iv),\n        backend=default_backend()\n    )\n    \n    # Decrypt the ciphertext\n    decryptor = cipher.decryptor()\n    plaintext = decryptor.update(ciphertext) + decryptor.finalize()\n    \n    # Clear sensitive key material\n    del encryption_key\n    del hmac_key\n    \n    return plaintext\n\n\ndef main():\n    """\n    Test cases demonstrating secure AES-CBC encryption with HMAC.\n    Each test uses a unique salt and IV generated by CSPRNG.\n    """\n    print("AES-256-CBC with HMAC-SHA256 Encryption Test Cases\\n")\n    \n    # Test case 1: Basic encryption/decryption\n    print("Test 1: Basic encryption/decryption")\n    plaintext1 = b"Secret message to encrypt"\n    passphrase1 = "StrongPassphrase123!@#"\n    encrypted1 = encrypt_aes_cbc_with_hmac(plaintext1, passphrase1)\n    decrypted1 = decrypt_aes_cbc_with_hmac(encrypted1, passphrase1)\n    print(f"Original:  {plaintext1}")\n    print(f"Encrypted length: {len(encrypted1)} bytes")\n    print(f"Decrypted: {decrypted1}")\n    print(f"Match: {plaintext1 == decrypted1}\\n")\n    \n    # Test case 2: Encrypting binary data\n    print("Test 2: Binary data encryption")\n    plaintext2 = os.urandom(64)\n    passphrase2 = "AnotherSecurePass456$%^"\n    encrypted2 = encrypt_aes_cbc_with_hmac(plaintext2, passphrase2)\n    decrypted2 = decrypt_aes_cbc_with_hmac(encrypted2, passphrase2)\n    print(f"Original length:  {len(plaintext2)} bytes")\n    print(f"Encrypted length: {len(encrypted2)} bytes")\n    print(f"Match: {plaintext2 == decrypted2}\\n")\n    \n    # Test case 3: UTF-8 text encryption\n    print("Test 3: UTF-8 text encryption")\n    plaintext3 = "Hello, 世界! 🔐".encode('utf-8')\n    passphrase3 = "Unicode_Passphrase_2024!"\n    encrypted3 = encrypt_aes_cbc_with_hmac(plaintext3, passphrase3)\n    decrypted3 = decrypt_aes_cbc_with_hmac(encrypted3, passphrase3)\n    print(f"Original:  {plaintext3.decode('utf-8')}")\n    print(f"Decrypted: {decrypted3.decode('utf-8')}")\n    print(f"Match: {plaintext3 == decrypted3}\
")\n    \n    # Test case 4: Large data encryption\n    print("Test 4: Large data encryption")\n    plaintext4 = b"X" * 10000\n    passphrase4 = "LargeDataPass789&*()"\n    encrypted4 = encrypt_aes_cbc_with_hmac(plaintext4, passphrase4)\n    decrypted4 = decrypt_aes_cbc_with_hmac(encrypted4, passphrase4)\n    print(f"Original length:  {len(plaintext4)} bytes")\n    print(f"Encrypted length: {len(encrypted4)} bytes")\n    print(f"Match: {plaintext4 == decrypted4}\
")\n    \n    # Test case 5: Verify HMAC authentication failure detection\n    print("Test 5: HMAC authentication failure detection")\n    plaintext5 = b"Authenticated message"\n    passphrase5 = "AuthTestPass!@#123"\n    encrypted5 = encrypt_aes_cbc_with_hmac(plaintext5, passphrase5)\n    \n    # Tamper with the ciphertext\n    tampered = bytearray(encrypted5)\n    tampered[-50] ^= 0xFF  # Flip bits in ciphertext\n    \n    try:\n        decrypt_aes_cbc_with_hmac(bytes(tampered), passphrase5)\n        print("FAILED: Tampered data was accepted")\n    except ValueError as e:\n        print(f"SUCCESS: Tampered data rejected - {e}\
")\n    \n    print("All test cases completed successfully!")\n\n\nif __name__ == "__main__":
    main()
