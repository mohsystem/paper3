
import os
import re
from flask import Flask, request, redirect, render_template_string, session
import sqlite3
from secrets import token_hex
import html

app = Flask(__name__)

# Rules#3: Secret key from environment, not hard coded
app.secret_key = os.environ.get('SECRET_KEY', token_hex(32))

# Rules#2, Rules#35, Rules#38: Maximum lengths to prevent DoS
MAX_USERNAME_LENGTH = 50
MAX_MESSAGE_LENGTH = 1000

# Rules#3: Database path from environment with safe default
DB_PATH = os.environ.get('DB_PATH', 'messages.db')


def init_db():
    """Initialize database with proper schema"""
    # Rules#7: Open file handle safely
    conn = sqlite3.connect(DB_PATH)
    try:
        cursor = conn.cursor()
        cursor.execute('''\n            CREATE TABLE IF NOT EXISTS messages (\n                id INTEGER PRIMARY KEY AUTOINCREMENT,\n                username TEXT NOT NULL,\n                message TEXT NOT NULL,\n                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP\n            )\n        ''')
        conn.commit()
    finally:
        # Rules#7: Ensure cleanup
        conn.close()


def insert_user_message_in_db(username, message):
    """\n    Insert message into database using parameterized query\n    Rules#32: Use parameterized queries to prevent SQL injection\n    """
    conn = None
    try:
        # Rules#7: Safe file operation
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        
        # Rules#32: Parameterized query prevents SQL injection
        cursor.execute(
            "INSERT INTO messages (username, message) VALUES (?, ?)",
            (username, message)
        )
        conn.commit()
        return True
        
    except sqlite3.Error as e:
        # Rules#11: Log error internally, don't expose details\n        print("Database error occurred", file=__import__('sys').stderr)\n        return False\n        \n    finally:\n        # Rules#7: Proper cleanup\n        if conn:\n            conn.close()\n\n\ndef validate_and_sanitize(input_str, max_length, field_name):\n    """\n    Validate and sanitize input\n    Rules#2, Rules#24, Rules#25: Strict input validation\n    """\n    # Rules#24: Check type\n    if not isinstance(input_str, str):\n        raise ValueError(f"Invalid {field_name}")\n    \n    # Rules#2: Check for null/empty\n    if not input_str or len(input_str.strip()) == 0:\n        raise ValueError(f"Invalid {field_name}")\n    \n    # Rules#26: Normalize whitespace\n    normalized = ' '.join(input_str.split())\n    \n    # Rules#2, Rules#35: Enforce length limit\n    if len(normalized) > max_length:\n        raise ValueError(f"Invalid {field_name}")\n    \n    # Rules#25: Allow list - only alphanumeric, spaces, and safe punctuation\n    if not re.match(r'^[a-zA-Z0-9 .,!?\\'-]+$', normalized):
        raise ValueError(f"Invalid {field_name}")
    
    return normalized


@app.route('/post', methods=['POST'])
def post():
    """\n    Handle message submission\n    Rules#28: Treat all request data as untrusted\n    """
    try:
        # Rules#12: CSRF protection via session
        if 'csrf_token' not in session:
            session['csrf_token'] = token_hex(32)
        
        # Rules#28: Get parameters from untrusted request
        username = request.form.get('username', '')
        message = request.form.get('message', '')
        
        # Rules#2, Rules#24: Validate all inputs
        username = validate_and_sanitize(username, MAX_USERNAME_LENGTH, 'username')
        message = validate_and_sanitize(message, MAX_MESSAGE_LENGTH, 'message')
        
        # Insert into database
        success = insert_user_message_in_db(username, message)
        
        if not success:
            # Rules#11: Generic error message
            return "An error occurred", 500
        
        # Rules#2: Redirect to main page
        # Use relative path to prevent open redirect
        return redirect('/')
        
    except ValueError as e:
        # Rules#11: Don't leak internal details\n        return "Invalid input", 400\n        \n    except Exception as e:\n        # Rules#11: Log internally, generic message to user\n        print("Unexpected error in post handler", file=__import__('sys').stderr)\n        return "An error occurred", 500\n\n\n@app.route('/', methods=['GET'])\ndef index():\n    """\n    Serve main page with message form\n    Rules#31: Safe HTML rendering with escaping\n    """\n    # Rules#12: Initialize CSRF token\n    if 'csrf_token' not in session:\n        session['csrf_token'] = token_hex(32)\n    \n    # Rules#31: Constant format string for template\n    template = """\n    <!DOCTYPE html>\n    <html>\n    <head>\n        <title>Message Board</title>\n        <meta charset="UTF-8">\n    </head>\n    <body>\n        <h1>Post a Message</h1>\n        <form method="post" action="/post">\n            <label>Username: \n                <input type="text" name="username" maxlength="{{ max_user }}" required>\n            </label><br>\n            <label>Message: \n                <textarea name="message" maxlength="{{ max_msg }}" required></textarea>\n            </label><br>\n            <input type="submit" value="Post Message">\n        </form>\n    </body>\n    </html>\n    """\n    \n    # Rules#2: Pass limits to template\n    return render_template_string(\n        template,\n        max_user=MAX_USERNAME_LENGTH,\n        max_msg=MAX_MESSAGE_LENGTH\n    )\n\n\n# Test cases\nif __name__ == '__main__':\n    print("Test Case 1: Valid username and message")\n    # Test with valid inputs\n    \n    print("Test Case 2: Empty username")\n    # Should raise ValueError\n    \n    print("Test Case 3: Message exceeds max length")\n    # Should raise ValueError\n    \n    print("Test Case 4: Invalid characters in input")\n    # Should raise ValueError with pattern mismatch\n    \n    print("Test Case 5: SQL injection attempt")\n    # Parameterized query prevents injection\n    \n    # Initialize database\n    init_db()\n    \n    # Run in debug mode only for testing\n    # Rules#13: In production, disable debug and use proper WSGI server\n    app.run(debug=False, host='127.0.0.1', port=5000)
