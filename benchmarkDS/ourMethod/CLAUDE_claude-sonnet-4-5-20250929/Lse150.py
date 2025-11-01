
from flask import Flask, request, redirect, url_for, render_template_string, session
import secrets
import re
import logging
from typing import Optional
import sqlite3
from functools import wraps

app = Flask(__name__)

# Rules#3: Secret key must come from environment, not hard coded
# In production: app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY')
app.config['SECRET_KEY'] = secrets.token_hex(32)  # For demonstration only

# Rules#2: Maximum lengths to prevent resource exhaustion
MAX_USERNAME_LENGTH = 50
MAX_MESSAGE_LENGTH = 500

# Rules#11: Generic error message
GENERIC_ERROR = "Unable to process request"

# Rules#10: Configure secure logging with constant format strings
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def validate_username(username: Optional[str]) -> bool:
    """\n    Rules#2: Validates username input\n    Checks type, length, and allowed character set\n    """
    if not username or not isinstance(username, str):
        return False
    
    # Rules#2: Length validation
    if len(username) > MAX_USERNAME_LENGTH:
        return False
    
    # Rules#2: Allow only alphanumeric, underscore, hyphen
    if not re.match(r'^[a-zA-Z0-9_-]+$', username):
        return False
    
    return True

def validate_message(message: Optional[str]) -> bool:
    """\n    Rules#2: Validates message input\n    Checks type, length, and encoding\n    """
    if not message or not isinstance(message, str):
        return False
    
    # Rules#2: Length check to prevent excessive resource use
    if len(message) > MAX_MESSAGE_LENGTH:
        return False
    
    return True

def insert_user_message_in_db(username: str, message: str) -> None:
    """\n    Rules#32: Uses parameterized query to prevent SQL injection\n    Rules#2: All inputs already validated before calling this function\n    """
    # Rules#32: Use parameterized query with ? placeholders
    # Never concatenate user input into SQL
    query = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, datetime('now'))"
    
    try:
        # Rules#8: Use context manager for automatic resource cleanup
        with sqlite3.connect('messages.db') as conn:
            cursor = conn.cursor()
            # Rules#32: Parameters passed separately, safely escaped by sqlite3
            cursor.execute(query, (username, message))
            conn.commit()
    except sqlite3.Error as e:
        # Rules#10-11: Log detailed error securely, raise generic exception
        logger.error("Database error: %s", str(type(e).__name__))
        raise RuntimeError(GENERIC_ERROR)

def csrf_protect(f):
    """\n    Rules#24-28: CSRF protection decorator\n    """
    @wraps(f)
    def decorated_function(*args, **kwargs):
        # Rules#28: Validate CSRF token from session and request
        session_token = session.get('csrf_token')
        request_token = request.form.get('csrf_token')
        
        if not session_token or not request_token:
            # Rules#11: Generic error without details
            logger.warning("CSRF token missing")
            return GENERIC_ERROR, 403
        
        # Rules#24: Constant-time comparison to prevent timing attacks
        if not secrets.compare_digest(session_token, request_token):
            logger.warning("CSRF token mismatch")
            return GENERIC_ERROR, 403
        
        return f(*args, **kwargs)
    return decorated_function

@app.route('/post', methods=['POST'])
@csrf_protect
def post():
    """\n    POST handler for message submission\n    Rules#24-28: Comprehensive input validation\n    Rules#11: Error handling without leaking details\n    """
    # Rules#28: Treat all input as untrusted
    username = request.form.get('username', '')
    message = request.form.get('message', '')
    
    # Rules#24-25: Validate with allow list approach
    if not validate_username(username):
        return "Invalid username", 400
    
    if not validate_message(message):
        return "Invalid message", 400
    
    try:
        # Rules#32: Call function that uses parameterized queries
        insert_user_message_in_db(username, message)
        
        # Rules#11: Redirect on success without leaking details
        return redirect(url_for('index'))
    
    except RuntimeError as e:
        # Rules#11: Return generic error to user
        return str(e), 500

@app.route('/', methods=['GET'])
def index():
    """\n    GET handler to display the form\n    Rules#5: Generate CSRF token with CSPRNG\n    Rules#24: Include CSRF token in form\n    """
    # Rules#5: Generate CSRF token using secrets module (CSPRNG)
    if 'csrf_token' not in session:
        session['csrf_token'] = secrets.token_hex(32)
    
    csrf_token = session['csrf_token']
    
    # Rules#24: HTML template with CSRF token and input validation
    # Rules#2: maxlength attributes enforce client-side limits
    template = """\n    <!DOCTYPE html>\n    <html>\n    <head>\n        <title>Message Board</title>\n    </head>\n    <body>\n        <h1>Post a Message</h1>\n        <form method="POST" action="{{ url_for('post') }}">\n            <input type="hidden" name="csrf_token" value="{{ csrf_token }}">\n            <label>Username: \n                <input type="text" name="username" maxlength="{{ max_user }}" required>\n            </label><br>\n            <label>Message: \n                <textarea name="message" maxlength="{{ max_msg }}" required></textarea>\n            </label><br>\n            <button type="submit">Submit</button>\n        </form>\n    </body>\n    </html>\n    """
    
    # Rules#10: Use Jinja2's auto-escaping, pass data as parameters\n    return render_template_string(\n        template,\n        csrf_token=csrf_token,\n        max_user=MAX_USERNAME_LENGTH,\n        max_msg=MAX_MESSAGE_LENGTH\n    )\n\ndef init_db():\n    """Initialize database with proper schema"""\n    with sqlite3.connect('messages.db') as conn:\n        cursor = conn.cursor()\n        cursor.execute("""\n            CREATE TABLE IF NOT EXISTS messages (\n                id INTEGER PRIMARY KEY AUTOINCREMENT,\n                username TEXT NOT NULL,\n                message TEXT NOT NULL,\n                created_at TEXT NOT NULL\n            )\n        """)\n        conn.commit()\n\n# Test cases\nif __name__ == '__main__':\n    # Initialize database\n    init_db()\n    \n    # Test 1: Valid username\n    print("Test 1:", validate_username("john_doe"))  # True\n    \n    # Test 2: Invalid username with special chars\n    print("Test 2:", validate_username("john@doe"))  # False\n    \n    # Test 3: Username too long\n    print("Test 3:", validate_username("a" * 100))  # False\n    \n    # Test 4: Valid message\n    print("Test 4:", validate_message("Hello world!"))  # True\n    \n    # Test 5: Message too long\n    print("Test 5:", validate_message("a" * 600))  # False\n    \n    # Rules#6: In production, use HTTPS and proper WSGI server\n    # app.run(ssl_context='adhoc') for development HTTPS
    app.run(debug=False)
