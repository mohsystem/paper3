
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
import base64
import secrets

class Task72:
    # SECURITY WARNING: Static IV is a critical vulnerability in production!
    # This should NEVER be used in real applications.
    # Each encryption should use a unique, randomly generated IV.
    STATIC_IV = bytes(16)  # All zeros - INSECURE!
    
    @staticmethod
    def encrypt_data(data, key):
        """\n        Encrypts data using AES-CBC with PKCS7 padding\n        WARNING: Uses static IV - NOT SECURE for production use!\n        \n        :param data: The plaintext string to encrypt\n        :param key: The encryption key (16, 24, or 32 bytes)\n        :return: Base64 encoded encrypted data\n        """
        if not data:
            raise ValueError("Data cannot be empty")
        
        if not key or len(key) not in [16, 24, 32]:
            raise ValueError("Key must be 16, 24, or 32 bytes for AES")
        
        # Step 1: Use static IV (class variable)
        # Step 2: Initialize cipher with AES in CBC mode
        cipher = AES.new(key, AES.MODE_CBC, Task72.STATIC_IV)
        
        # Step 3: Pad the input data to match block size (16 bytes for AES)
        data_bytes = data.encode('utf-8')
        padded_data = pad(data_bytes, AES.block_size)
        
        # Step 4: Encrypt the padded data
        encrypted_bytes = cipher.encrypt(padded_data)
        
        # Step 5: Return encrypted data as Base64 string
        return base64.b64encode(encrypted_bytes).decode('utf-8')
    
    @staticmethod
    def generate_secure_key(key_size=256):
        """\n        Generates a secure random key for AES encryption\n        \n        :param key_size: Key size in bits (128, 192, or 256)\n        :return: Random key bytes\n        """
        return secrets.token_bytes(key_size // 8)


def main():
    print("=== AES-CBC Encryption Demo ===")
    print("WARNING: This uses a static IV - NOT SECURE!\\n")
    
    try:
        # Generate a secure key for testing
        key = Task72.generate_secure_key(256)
        
        # Test Case 1: Simple text
        test1 = "Hello, World!"
        encrypted1 = Task72.encrypt_data(test1, key)
        print(f"Test 1 - Input: {test1}")
        print(f"Encrypted: {encrypted1}\\n")
        
        # Test Case 2: Longer text
        test2 = "This is a longer message that will span multiple blocks."
        encrypted2 = Task72.encrypt_data(test2, key)
        print(f"Test 2 - Input: {test2}")
        print(f"Encrypted: {encrypted2}\\n")
        
        # Test Case 3: Special characters
        test3 = "Special chars: !@#$%^&*()_+-={}[]|:;<>?,./\\\\"\n        encrypted3 = Task72.encrypt_data(test3, key)\n        print(f"Test 3 - Input: {test3}")\n        print(f"Encrypted: {encrypted3}\
")\n        \n        # Test Case 4: Unicode text\n        test4 = "Unicode: 你好世界 🌍"\n        encrypted4 = Task72.encrypt_data(test4, key)\n        print(f"Test 4 - Input: {test4}")\n        print(f"Encrypted: {encrypted4}\
")\n        \n        # Test Case 5: Demonstrating static IV vulnerability\n        test5a = "Same message"\n        test5b = "Same message"\n        encrypted5a = Task72.encrypt_data(test5a, key)\n        encrypted5b = Task72.encrypt_data(test5b, key)\n        print("Test 5 - Demonstrating Static IV Vulnerability:")\n        print(f"Message A: {test5a}")\n        print(f"Encrypted A: {encrypted5a}")\n        print(f"Message B: {test5b}")\n        print(f"Encrypted B: {encrypted5b}")\n        print(f"Same ciphertext? {encrypted5a == encrypted5b}")\n        print("^ This is a CRITICAL security flaw!")\n        \n    except Exception as e:\n        print(f"Encryption error: {e}")\n\n\nif __name__ == "__main__":
    main()
