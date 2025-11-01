
import os
import sys
import hashlib
import secrets
import sqlite3
from typing import Optional

# Constants for secure password hashing
SALT_LENGTH = 16  # 128 bits
ITERATIONS = 210000  # OWASP recommendation for PBKDF2
KEY_LENGTH = 32  # 256 bits

def generate_salt() -> bytes:
    """Generate cryptographically secure random salt - CWE-759, CWE-330"""
    return secrets.token_bytes(SALT_LENGTH)

def hash_password(password: str, salt: bytes) -> str:
    """Hash password using PBKDF2-HMAC-SHA256 - secure KDF - CWE-327"""
    # Use PBKDF2 with SHA-256 and high iteration count
    hash_bytes = hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt, ITERATIONS, KEY_LENGTH)
    
    # Combine salt and hash for storage
    combined = salt + hash_bytes
    return combined.hex()

def validate_password_policy(password: str) -> bool:
    """Enforce strong password policy - CWE-521"""
    if password is None or len(password) < 12 or len(password) > 128:
        return False
    
    # Check complexity requirements
    has_upper = any(c.isupper() for c in password)
    has_lower = any(c.islower() for c in password)
    has_digit = any(c.isdigit() for c in password)
    has_special = any(c in "!@#$%^&*()_+-=[]{}; ':\\"\\\\|,.<>/?" for c in password)
    
    return has_upper and has_lower and has_digit and has_special

def register_user(username: str, password: str, conn: sqlite3.Connection) -> bool:
    """Register user with secure password hashing"""
    # Input validation - CWE-20
    if username is None or not username.strip() or len(username) > 100:
        print("Invalid username", file=sys.stderr)
        return False
    
    # Validate password policy - CWE-521
    if not validate_password_policy(password):
        print("Password must be 12-128 characters with uppercase, lowercase, digit, and special character", 
              file=sys.stderr)
        return False
    
    try:
        # Generate unique salt for this user - CWE-759, CWE-330
        salt = generate_salt()
        
        # Hash password with salt using secure KDF - CWE-327
        hashed_password = hash_password(password, salt)
        
        # Use parameterized query to prevent SQL injection - CWE-89
        cursor = conn.cursor()
        cursor.execute("INSERT INTO users (username, password_hash) VALUES (?, ?)", 
                      (username, hashed_password))
        conn.commit()
        return True
        
    except sqlite3.IntegrityError:
        # Generic error message - don't leak database details - CWE-209\n        print("Registration failed: username may already exist", file=sys.stderr)\n        return False\n    except Exception as e:\n        # Generic error message - CWE-209\n        print("Registration failed due to system error", file=sys.stderr)\n        return False\n\ndef main():\n    """Main function with test cases"""\n    # Create database connection - using SQLite for demonstration\n    # In production, use proper database with TLS - CWE-319\n    conn = sqlite3.connect(':memory:')\n    \n    # Create users table\n    cursor = conn.cursor()\n    cursor.execute('''CREATE TABLE users 
                     (id INTEGER PRIMARY KEY AUTOINCREMENT,
                      username TEXT UNIQUE NOT NULL,
                      password_hash TEXT NOT NULL)''')\n    conn.commit()\n    \n    # Test case 1: Valid registration\n    print(f"Test 1: {register_user('alice', 'SecurePass123!@#', conn)}")\n    \n    # Test case 2: Valid registration with different user\n    print(f"Test 2: {register_user('bob', 'MyP@ssw0rd2024!', conn)}")\n    \n    # Test case 3: Weak password (too short)\n    print(f"Test 3: {register_user('charlie', 'Short1!', conn)}")\n    \n    # Test case 4: Password without special character\n    print(f"Test 4: {register_user('david', 'NoSpecialChar123', conn)}")\n    \n    # Test case 5: Empty username\n    print(f"Test 5: {register_user('', 'ValidPass123!@#', conn)}")\n    \n    conn.close()\n\nif __name__ == "__main__":
    main()
