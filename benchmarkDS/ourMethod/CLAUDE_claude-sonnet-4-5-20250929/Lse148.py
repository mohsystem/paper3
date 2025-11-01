
import sqlite3
import os
import html
import secrets
from typing import Dict, List, Optional
from datetime import datetime

# Rule#2: Define maximum input lengths
MAX_USERNAME_LENGTH = 50
MAX_MESSAGE_LENGTH = 500

def post(request: Dict[str, str], session: Dict[str, str]) -> str:
    """\n    Handle POST request for message submission.\n    Rule#2: Validate all inputs before processing.\n    Rule#32: Use parameterized queries for database operations.\n    """
    try:
        # Rule#2: Validate CSRF token to prevent cross-site request forgery
        csrf_token = request.get('csrf_token')
        session_token = session.get('csrf_token')
        
        if not csrf_token or not session_token or csrf_token != session_token:
            # Rule#11: Return generic error message
            return 'error'
        
        # Rule#2: Get and validate inputs
        username = request.get('username')
        message = request.get('message')
        
        if not username or not message:
            return 'error'
        
        # Rule#2: Validate length constraints
        if len(username) == 0 or len(username) > MAX_USERNAME_LENGTH:
            return 'error'
        if len(message) == 0 or len(message) > MAX_MESSAGE_LENGTH:
            return 'error'
        
        # Rule#2: Validate encoding - ensure valid UTF-8
        try:
            username.encode('utf-8')
            message.encode('utf-8')
        except UnicodeError:
            return 'error'
        
        # Rule#2: Check for control characters
        if not is_valid_input(username) or not is_valid_input(message):
            return 'error'
        
        # Insert into database using safe parameterized query
        insert_user_message_in_db(username, message)
        
        # Redirect to main page
        return 'redirect:/main'
        
    except Exception as e:
        # Rule#11: Do not leak internal details
        # Rule#10: Log without sensitive data
        print("Error processing post request", file=__import__('sys').stderr)
        return 'error'

def is_valid_input(text: str) -> bool:
    """\n    Rule#2: Validate input contains no dangerous control characters.\n    """
    for char in text:
        # Allow newline and tab, reject other control characters
        if char.isprintable() or char in '\\n\\t':
            continue
        return False
    return True

def insert_user_message_in_db(username: str, message: str) -> None:
    """\n    Rule#32: Use parameterized queries to prevent SQL injection.\n    Rule#3: Read database path from environment variable.\n    """
    # Rule#3: Get database path from environment, not hardcoded
    db_path = os.environ.get('DB_PATH', 'messages.db')
    
    # Rule#32: Parameterized query prevents SQL injection
    sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, ?)"
    
    # Use context manager for automatic cleanup
    with sqlite3.connect(db_path) as conn:
        cursor = conn.cursor()
        # Rule#32: Use parameter binding, never string concatenation
        cursor.execute(sql, (username, message, datetime.now().isoformat()))
        conn.commit()

def get_messages() -> List[Dict[str, str]]:
    """\n    Retrieve messages from database with XSS protection.\n    Rule#2: Escape output to prevent XSS attacks.\n    """
    messages = []
    
    try:
        db_path = os.environ.get('DB_PATH', 'messages.db')
        
        # Rule#32: Safe parameterized query
        sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC LIMIT 100"
        
        with sqlite3.connect(db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(sql)
            
            for row in cursor.fetchall():
                # Rule#2: Escape HTML to prevent XSS
                messages.append({
                    'username': html.escape(row[0] or ''),
                    'message': html.escape(row[1] or ''),
                    'timestamp': html.escape(row[2] or '')
                })
                
    except Exception as e:
        # Rule#11: Generic error handling
        print("Error retrieving messages", file=__import__('sys').stderr)
    
    return messages

def generate_csrf_token() -> str:
    """\n    Rule#5: Generate CSRF token using cryptographically secure random.\n    """
    # Use secrets module for cryptographic randomness
    return secrets.token_urlsafe(32)

def main():
    """Test cases demonstrating secure usage."""
    # Setup: Create in-memory database for testing
    import sqlite3
    db_path = ':memory:'
    os.environ['DB_PATH'] = db_path
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute('''\n        CREATE TABLE messages (\n            id INTEGER PRIMARY KEY AUTOINCREMENT,\n            username TEXT,\n            message TEXT,\n            created_at TEXT\n        )\n    ''')
    conn.commit()
    conn.close()
    
    # Test case 1: Valid post
    token1 = generate_csrf_token()
    req1 = {'username': 'alice', 'message': 'Hello world!', 'csrf_token': token1}
    session1 = {'csrf_token': token1}
    print(f"Test 1: {post(req1, session1)}")
    
    # Test case 2: Another valid post
    token2 = generate_csrf_token()
    req2 = {'username': 'bob', 'message': 'Test message', 'csrf_token': token2}
    session2 = {'csrf_token': token2}
    print(f"Test 2: {post(req2, session2)}")
    
    # Test case 3: Invalid CSRF token
    req3 = {'username': 'eve', 'message': 'Attack', 'csrf_token': 'wrong'}
    session3 = {'csrf_token': 'correct'}
    print(f"Test 3 (should error): {post(req3, session3)}")
    
    # Test case 4: Oversized input
    token4 = generate_csrf_token()
    req4 = {'username': 'x' * 100, 'message': 'msg', 'csrf_token': token4}
    session4 = {'csrf_token': token4}
    print(f"Test 4 (should error): {post(req4, session4)}")
    
    # Test case 5: XSS attempt
    token5 = generate_csrf_token()
    req5 = {'username': 'hacker', 'message': "<script>alert('xss')</script>", 'csrf_token': token5}
    session5 = {'csrf_token': token5}
    print(f"Test 5: {post(req5, session5)}")
    
    # Display all messages
    print("\\nMessages:")
    messages = get_messages()
    for msg in messages:
        print(f"{msg['username']}: {msg['message']}")

if __name__ == '__main__':
    main()
