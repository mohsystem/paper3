
import re
import subprocess
import sqlite3
from typing import Optional

# Email validation pattern (RFC 5322 simplified)
EMAIL_PATTERN = re.compile(
    r'^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$'
)

# Maximum email length to prevent buffer overflow
MAX_EMAIL_LENGTH = 254


def unsubscribe_email(email: Optional[str], db_path: str, script_path: str) -> int:
    """\n    Checks if email exists in database and unsubscribes if found.\n    Returns 1 if unsubscribe successful, 0 otherwise.\n    \n    Args:\n        email: The email address to unsubscribe\n        db_path: Path to SQLite database\n        script_path: Path to unsubscribe script\n        \n    Returns:\n        1 if successful, 0 otherwise\n    """
    # Input validation: check for None and empty
    if email is None or not isinstance(email, str) or not email.strip():
        return 0
    
    email = email.strip()
    
    # Validate email length to prevent buffer overflow
    if len(email) > MAX_EMAIL_LENGTH:
        return 0
    
    # Validate email format to prevent injection
    if not EMAIL_PATTERN.match(email):
        return 0
    
    # Validate database and script paths
    if db_path is None or script_path is None:
        return 0
    
    # Validate script path to prevent path traversal
    if '..' in script_path or '~' in script_path:
        return 0
    
    conn = None
    cursor = None
    
    try:
        # Connect to database with isolation level to prevent race conditions
        conn = sqlite3.connect(db_path, timeout=5.0, isolation_level='DEFERRED')
        cursor = conn.cursor()
        
        # Use parameterized query to prevent SQL injection
        query = "SELECT COUNT(*) FROM subscribers WHERE email = ?"
        cursor.execute(query, (email,))
        
        result = cursor.fetchone()
        
        if result and result[0] > 0:
            # Email exists, call unsubscribe script
            return call_unsubscribe_script(email, script_path)
        else:
            # Email doesn't exist\n            return 0\n            \n    except sqlite3.Error:\n        # Log error securely without exposing sensitive details\n        print("Database error occurred", file=__import__('sys').stderr)\n        return 0\n    finally:\n        # Close resources\n        if cursor:\n            cursor.close()\n        if conn:\n            conn.close()\n\n\ndef call_unsubscribe_script(email: str, script_path: str) -> int:\n    """\n    Calls the unsubscribe script with validated email parameter\n    \n    Args:\n        email: Validated email address\n        script_path: Path to unsubscribe script\n        \n    Returns:\n        1 if script returns 0, otherwise 0\n    """\n    try:\n        # Use subprocess with list to prevent command injection\n        # Pass email as separate argument, not concatenated into command\n        result = subprocess.run(\n            [script_path, email],\n            capture_output=True,\n            timeout=5,\n            check=False,\n            env={},  # Clear environment variables\n            text=True\n        )\n        \n        # Return 1 if script returns 0 (success), otherwise 0\n        return 1 if result.returncode == 0 else 0\n        \n    except subprocess.TimeoutExpired:\n        # Script took too long\n        print("Script execution timeout", file=__import__('sys').stderr)\n        return 0\n    except Exception:\n        # Log generic error without exposing details\n        print("Script execution error occurred", file=__import__('sys').stderr)\n        return 0\n\n\nif __name__ == "__main__":\n    # Test cases with safe mock values\n    db_path = ":memory:"\n    script_path = "/usr/local/bin/unsubscribe.sh"\n    \n    # Test case 1: Valid email format\n    print(f"Test 1 (valid email): {unsubscribe_email('user@example.com', db_path, script_path)}")\n    \n    # Test case 2: None email\n    print(f"Test 2 (None email): {unsubscribe_email(None, db_path, script_path)}")\n    \n    # Test case 3: Empty email\n    print(f"Test 3 (empty email): {unsubscribe_email('', db_path, script_path)}")\n    \n    # Test case 4: Invalid email format (SQL injection attempt)\n    print(f"Test 4 (injection attempt): {unsubscribe_email(\\"' OR '1'='1\\", db_path, script_path)}")\n    \n    # Test case 5: Email too long\n    long_email = 'a' * 300 + '@example.com'
    print(f"Test 5 (too long): {unsubscribe_email(long_email, db_path, script_path)}")
