
import os
import re
import sqlite3
import secrets
import hashlib
from typing import Tuple

# Rules#34: Validate and sanitize every external input
USERNAME_PATTERN = re.compile(r'^[a-zA-Z0-9_]{3,30}$')
EMAIL_PATTERN = re.compile(r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$')
NAME_PATTERN = re.compile(r'^[a-zA-Z\\s\\'-]{1,50}$')
MIN_PASSWORD_LENGTH = 8
MAX_PASSWORD_LENGTH = 128
PBKDF2_ITERATIONS = 210000
SALT_LENGTH = 16

def validate_username(username: str) -> bool:
    """Rules#2: Check type, length, range for username"""
    if not username or not isinstance(username, str):
        return False
    return bool(USERNAME_PATTERN.match(username))

def validate_password(password: str) -> bool:
    """Rules#2: Validate password length"""
    if not password or not isinstance(password, str):
        return False
    length = len(password)
    return MIN_PASSWORD_LENGTH <= length <= MAX_PASSWORD_LENGTH

def validate_name(name: str) -> bool:
    """Rules#2: Validate first/last name"""
    if not name or not isinstance(name, str):
        return False
    return bool(NAME_PATTERN.match(name))

def validate_email(email: str) -> bool:
    """Rules#2: Validate email format and length"""
    if not email or not isinstance(email, str):
        return False
    return bool(EMAIL_PATTERN.match(email)) and len(email) <= 254

def hash_password(password: str, salt: bytes) -> str:
    """Rules#5: Use PBKDF2-HMAC-SHA-256 with 210000 iterations and unique salt"""
    key = hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt, PBKDF2_ITERATIONS)
    return key.hex()

def generate_salt() -> bytes:
    """Rules#5: Generate nonces with CSPRNG using secrets module"""
    return secrets.token_bytes(SALT_LENGTH)

def get_db_path() -> str:
    """Rules#3: Read DB path from environment, not hard coded"""
    return os.getenv('DB_PATH', 'users.db')

def register_user(username: str, password: str, first_name: str, 
                  last_name: str, email: str) -> Tuple[bool, str]:
    """\n    Register a new user with validated inputs and hashed password.\n    \n    Rules#34-42: All inputs validated at trust boundary before use.\n    Rules#42: Parameterized queries prevent SQL injection.\n    Rules#11: Generic error messages returned to caller.\n    """
    # Rules#34: Validate all inputs early
    if not validate_username(username):
        return False, "Registration failed"
    if not validate_password(password):
        return False, "Registration failed"
    if not validate_name(first_name):
        return False, "Registration failed"
    if not validate_name(last_name):
        return False, "Registration failed"
    if not validate_email(email):
        return False, "Registration failed"
    
    conn = None
    try:
        # Rules#3: Get database path from environment
        db_path = get_db_path()
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Rules#42: Use parameterized query to prevent SQL injection
        cursor.execute("SELECT COUNT(*) FROM users WHERE username = ?", (username,))
        count = cursor.fetchone()[0]
        
        if count > 0:
            # Rules#11: Generic error - no info about existing username leaked
            return False, "Registration failed"
        
        # Rules#5: Generate salt with CSPRNG and hash password
        salt = generate_salt()
        password_hash = hash_password(password, salt)
        
        # Rules#42: Parameterized insert prevents SQL injection
        cursor.execute(
            "INSERT INTO users (username, password_hash, salt, first_name, last_name, email) VALUES (?, ?, ?, ?, ?, ?)",
            (username, password_hash, salt.hex(), first_name, last_name, email)
        )
        conn.commit()
        
        return True, "Registration succeeded"
        
    except sqlite3.IntegrityError:
        # Rules#11: Generic error message to caller
        return False, "Registration failed"
    except Exception as e:
        # Rules#11: Log error class but return generic message
        print(f"Registration error: {type(e).__name__}", file=__import__('sys').stderr)
        return False, "Registration failed"
    finally:
        if conn:
            conn.close()

def init_database():
    """Initialize database table for testing"""
    try:
        db_path = get_db_path()
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute('''\n            CREATE TABLE IF NOT EXISTS users (\n                id INTEGER PRIMARY KEY AUTOINCREMENT,\n                username TEXT UNIQUE NOT NULL,\n                password_hash TEXT NOT NULL,\n                salt TEXT NOT NULL,\n                first_name TEXT NOT NULL,\n                last_name TEXT NOT NULL,\n                email TEXT NOT NULL\n            )\n        ''')
        conn.commit()
        conn.close()
    except Exception as e:
        print(f"Database init error: {type(e).__name__}", file=__import__('sys').stderr)

if __name__ == "__main__":
    init_database()
    
    # Test case 1: Valid registration
    success, message = register_user("john_doe", "SecurePass123!", "John", "Doe", "john@example.com")
    print(f"Test 1 - Valid registration: {message}")
    
    # Test case 2: Duplicate username
    success, message = register_user("john_doe", "AnotherPass456!", "Jane", "Smith", "jane@example.com")
    print(f"Test 2 - Duplicate username: {message}")
    
    # Test case 3: Invalid username (too short)
    success, message = register_user("ab", "ValidPass789!", "Bob", "Johnson", "bob@example.com")
    print(f"Test 3 - Invalid username: {message}")
    
    # Test case 4: Invalid password (too short)
    success, message = register_user("alice_w", "short", "Alice", "Williams", "alice@example.com")
    print(f"Test 4 - Invalid password: {message}")
    
    # Test case 5: Invalid email format
    success, message = register_user("charlie_b", "ValidPass999!", "Charlie", "Brown", "invalid-email")
    print(f"Test 5 - Invalid email: {message}")
