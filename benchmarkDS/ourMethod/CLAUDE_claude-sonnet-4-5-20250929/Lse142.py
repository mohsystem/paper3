
import re
import sqlite3
import os
from typing import Optional

# Rules#2: Strict email validation pattern
EMAIL_PATTERN = re.compile(r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$')
MAX_EMAIL_LENGTH = 254  # RFC 5321 maximum

def unsubscribe(email: str) -> str:
    """\n    Unsubscribe function with security controls\n    Rules#2: Validates input type, length, and format\n    Rules#32: Uses parameterized queries to prevent SQL injection\n    Rules#11: Returns generic error messages\n    """
    # Rules#2: Validate input type
    if not isinstance(email, str):
        return "Invalid email address"
    
    # Rules#2: Validate length
    if len(email) > MAX_EMAIL_LENGTH:
        return "Invalid email address"
    
    # Rules#2: Validate format using allow list pattern
    if not EMAIL_PATTERN.match(email):
        return "Invalid email address"
    
    # Rules#3: Load database credentials from environment
    db_path = os.environ.get('DB_PATH', 'subscribers.db')
    
    conn = None
    cursor = None
    
    try:
        # Connect to database
        conn = sqlite3.connect(db_path, timeout=10.0)
        cursor = conn.cursor()
        
        # Rules#32: Use parameterized query to prevent SQL injection
        check_query = "SELECT COUNT(*) FROM subscribers WHERE email = ?"
        cursor.execute(check_query, (email,))
        result = cursor.fetchone()
        
        if result and result[0] > 0:
            # Email exists, remove it
            # Rules#32: Parameterized query prevents SQL injection
            delete_query = "DELETE FROM subscribers WHERE email = ?"
            cursor.execute(delete_query, (email,))
            conn.commit()
            
            # Rules#10: Do not log user email address
            if cursor.rowcount > 0:
                return "Successfully unsubscribed"
            else:
                return "Unsubscribe failed"
        else:
            # Email not in database
            return "Email address not found in database"
    
    except sqlite3.Error as e:
        # Rules#11: Log detailed error, return generic message
        print(f"Database error: {str(e)}", file=__import__('sys').stderr)
        if conn:
            conn.rollback()
        return "Service error occurred"
    
    finally:
        # Proper resource cleanup
        if cursor:
            cursor.close()
        if conn:
            conn.close()

def main():
    """Test cases demonstrating the unsubscribe function"""
    # Setup test database
    os.environ['DB_PATH'] = ':memory:'
    conn = sqlite3.connect(':memory:')
    cursor = conn.cursor()
    cursor.execute('CREATE TABLE subscribers (email TEXT PRIMARY KEY)')
    cursor.execute('INSERT INTO subscribers VALUES (?)', ('user@example.com',))
    conn.commit()
    conn.close()
    
    # Test cases
    print("Test 1 - Valid email:", unsubscribe("user@example.com"))
    print("Test 2 - Invalid format:", unsubscribe("invalid-email"))
    print("Test 3 - Empty string:", unsubscribe(""))
    print("Test 4 - Too long:", unsubscribe("a" * 300 + "@example.com"))
    print("Test 5 - SQL injection attempt:", unsubscribe("user@example.com' OR '1'='1"))

if __name__ == "__main__":
    main()
