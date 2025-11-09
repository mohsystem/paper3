
import secrets
import hashlib
from typing import Optional
from flask import Flask, request, session, redirect, url_for, render_template_string, abort

app = Flask(__name__)

# Rules#3: Never hard code secrets. Read from environment or secret manager.
# In production, set this via environment variable or secret manager
app.secret_key = secrets.token_hex(32)

# Rules#5: Use Argon2id or PBKDF2 with proper parameters for password hashing
# Simulated user database with hashed passwords (in production, use a proper database)
# Password: "secure_password123" hashed with PBKDF2-HMAC-SHA256
USERS = {
    "admin": {
        "password_hash": "d74ff0ee8da3b9806b18c877dbf29bbde50b5bd8e4dad7a3a725000feb82e8f1",
        "salt": "a1b2c3d4e5f6"
    }
}

# Rules#2: Input validation - validate all inputs
def validate_username(username: str) -> bool:
    """Validate username format and length."""
    if not username or not isinstance(username, str):
        return False
    # Allow only alphanumeric and underscore, length 3-20
    if len(username) < 3 or len(username) > 20:
        return False
    return username.replace('_', '').isalnum()

def validate_password(password: str) -> bool:
    """Validate password format and length."""
    if not password or not isinstance(password, str):
        return False
    # Enforce minimum length
    if len(password) < 8 or len(password) > 128:
        return False
    return True

def hash_password(password: str, salt: str) -> str:
    """Hash password using PBKDF2-HMAC-SHA256."""
    # Rules#5: Use PBKDF2 with at least 210000 iterations
    iterations = 210000
    password_bytes = password.encode('utf-8')
    salt_bytes = salt.encode('utf-8')
    dk = hashlib.pbkdf2_hmac('sha256', password_bytes, salt_bytes, iterations)
    return dk.hex()

def verify_password(username: str, password: str) -> bool:
    """Verify password against stored hash."""
    if username not in USERS:
        return False
    
    user_data = USERS[username]
    password_hash = hash_password(password, user_data["salt"])
    
    # Use constant-time comparison to prevent timing attacks
    return secrets.compare_digest(password_hash, user_data["password_hash"])

@app.route('/')
def index():
    """Index page with link to login."""
    # Rules#36: Use constant format strings for HTML rendering
    html = """\n    <!DOCTYPE html>\n    <html>\n    <head>\n        <title>Home</title>\n        <meta charset="UTF-8">\n    </head>\n    <body>\n        <h1>Welcome</h1>\n        <p><a href="/login">Login</a></p>\n    </body>\n    </html>\n    """
    return render_template_string(html)

@app.route('/login', methods=['GET', 'POST'])
def login():
    """Login page with form."""
    if request.method == 'GET':
        html = """\n        <!DOCTYPE html>\n        <html>\n        <head>\n            <title>Login</title>\n            <meta charset="UTF-8">\n        </head>\n        <body>\n            <h1>Login</h1>\n            <form method="POST" action="/login">\n                <label>Username: <input type="text" name="username" required maxlength="20"></label><br>\n                <label>Password: <input type="password" name="password" required maxlength="128"></label><br>\n                <button type="submit">Login</button>\n            </form>\n        </body>\n        </html>\n        """
        return render_template_string(html)
    
    return do_login()

def do_login():
    """Process login credentials."""
    # Rules#29: Validate and sanitize every external input
    username = request.form.get('username', '')
    password = request.form.get('password', '')
    
    # Rules#2: Input validation for type, length, format
    if not validate_username(username):
        # Rules#11: Generic error message, no details leaked
        return "Invalid credentials", 401
    
    if not validate_password(password):
        return "Invalid credentials", 401
    
    # Verify credentials
    if verify_password(username, password):
        # Rules#1: Regenerate session ID on authentication to prevent session fixation
        session.clear()
        session.permanent = False
        session['username'] = username
        
        return redirect(url_for('secret'))
    
    # Rules#11: Return generic message on failure
    return "Invalid credentials", 401

@app.route('/secret')
def secret():
    """Secret page accessible only after login."""
    # Rules#33: Treat session data as untrusted, validate it
    username = session.get('username')
    
    if not username or not validate_username(username):
        # Rules#1: Fail closed - deny access if session invalid
        abort(403)
    
    html = """\n    <!DOCTYPE html>\n    <html>\n    <head>\n        <title>Secret</title>\n        <meta charset="UTF-8">\n    </head>\n    <body>\n        <h1>Secret settings</h1>\n        <p>This is a protected page.</p>\n        <p><a href="/logout">Logout</a></p>\n    </body>\n    </html>\n    """
    return render_template_string(html)

@app.route('/logout')
def logout():
    """Logout and clear session."""
    # Rules#9: Clear sensitive session data
    session.clear()
    return redirect(url_for('index'))

# Rules#11: Custom error handlers that don't leak details\n@app.errorhandler(403)\ndef forbidden(e):\n    return "Access denied", 403\n\n@app.errorhandler(404)\ndef not_found(e):\n    return "Not found", 404\n\n@app.errorhandler(500)\ndef internal_error(e):\n    # Rules#11: Don't leak internal details
    return "Internal error", 500

if __name__ == '__main__':
    # Test cases
    print("Testing password hashing...")
    test_password = "secure_password123"
    test_salt = "a1b2c3d4e5f6"
    hashed = hash_password(test_password, test_salt)
    print(f"Hash: {hashed}")
    
    print("\\nTesting validation...")
    print(f"Valid username 'admin': {validate_username('admin')}")
    print(f"Invalid username 'ad': {validate_username('ad')}")
    print(f"Valid password: {validate_password('password123')}")
    print(f"Invalid password (short): {validate_password('short')}")
    
    print("\\nTesting password verification...")
    print(f"Correct credentials: {verify_password('admin', 'secure_password123')}")
    print(f"Wrong credentials: {verify_password('admin', 'wrongpassword')}")
    
    # Rules#13: In production, disable debug and use secure settings
    # app.run(debug=False, host='127.0.0.1', port=5000)
    print("\\nTo run the app: app.run(debug=False, host='127.0.0.1', port=5000)")
