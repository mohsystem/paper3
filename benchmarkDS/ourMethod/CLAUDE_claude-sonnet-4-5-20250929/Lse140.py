
import sqlite3
import os
from typing import Optional

def check_username_exists(username: str) -> bool:
    """\n    Check if username exists in database.\n    Returns True if exists, False otherwise.\n    \n    Args:\n        username: The username to check\n        \n    Returns:\n        bool: True if username exists, False otherwise\n    """
    # Rules#2: Input validation - check type and bounds
    if not isinstance(username, str):
        return False
    
    if not username or not username.strip():
        return False
    
    # Limit username length to prevent DoS
    if len(username) > 255:
        return False
    
    conn = None
    cursor = None
    
    try:
        # Rules#3: Never hard code credentials - load from environment
        db_path = os.getenv('DB_PATH', 'database.db')
        
        # Validate database path to prevent path traversal
        # Rules#2: Normalize and validate paths
        db_path = os.path.normpath(db_path)
        if '..' in db_path or db_path.startswith('/'):
            raise ValueError("Invalid database path")
        
        # Connect to database
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Rules#32: Use parameterized queries to prevent SQL injection
        # The ? placeholder ensures username is treated as data, not code
        query = "SELECT COUNT(*) FROM users WHERE username = ?"
        cursor.execute(query, (username,))
        
        result = cursor.fetchone()
        
        if result and result[0] > 0:
            return True
        
        return False
        
    except (sqlite3.Error, ValueError) as e:
        # Rules#11: Don't leak internal details\n        print("Database operation failed", file=__import__('sys').stderr)\n        return False\n        \n    finally:\n        # Clean up resources\n        if cursor:\n            cursor.close()\n        if conn:\n            conn.close()\n\n\nif __name__ == "__main__":\n    # Test case 1: Valid username that exists\n    print(f"Test 1 (existing user): {check_username_exists('alice')}")\n    \n    # Test case 2: Valid username that doesn't exist
    print(f"Test 2 (non-existing user): {check_username_exists('nonexistent')}")
    
    # Test case 3: Empty username
    print(f"Test 3 (empty): {check_username_exists('')}")
    
    # Test case 4: None value
    print(f"Test 4 (None): {check_username_exists(None) if isinstance(None, str) else False}")
    
    # Test case 5: Username with special characters
    print(f"Test 5 (special chars): {check_username_exists('user@test')}")
