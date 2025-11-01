
from flask import Flask, session, request
import secrets
import os

app = Flask(__name__)

# SECURITY: Use cryptographically secure random key generation
# In production, load from environment variable or secure vault
app.secret_key = os.environ.get('SECRET_KEY', secrets.token_hex(32))

# Configure secure session cookies
app.config.update(
    SESSION_COOKIE_SECURE=True,  # Only send over HTTPS
    SESSION_COOKIE_HTTPONLY=True,  # Prevent JavaScript access
    SESSION_COOKIE_SAMESITE='Strict',  # CSRF protection
    PERMANENT_SESSION_LIFETIME=1800  # 30 minute timeout
)

@app.route('/info', methods=['GET'])
def info():
    """\n    Returns user information from session.\n    SECURITY: Only accepts GET requests, validates session data,\n    and masks sensitive information before returning.\n    """
    # Validate that session exists and contains required data
    if 'username' not in session:
        # Return generic error, don't leak session details\n        return "Error: Unauthorized", 401\n    \n    # Retrieve session variables - treat all as untrusted input\n    username = session.get('username', '')\n    email = session.get('email', '')\n    ssn = session.get('ssn', '')\n    \n    # SECURITY: Validate and sanitize all session data\n    username = sanitize_for_output(username)\n    \n    # SECURITY: Mask sensitive information (email and SSN)\n    # Never return full SSN or email in responses\n    masked_email = mask_email(email)\n    masked_ssn = mask_ssn(ssn)\n    \n    # Return sanitized data using constant format string\n    # SECURITY: Never log sensitive data\n    return f"Username: {username}, Email: {masked_email}, SSN: {masked_ssn}"\n\n\ndef mask_email(email):\n    """\n    Masks email to show only first character and domain.\n    SECURITY: Prevents full email exposure in responses.\n    """\n    if not email or '@' not in email:\n        return "***"\n    \n    parts = email.split('@')\n    if len(parts) != 2 or not parts[0]:\n        return "***"\n    \n    return f"{parts[0][0]}***@{parts[1]}"\n\n\ndef mask_ssn(ssn):\n    """\n    Masks SSN to show only last 4 digits.\n    SECURITY: Prevents full SSN exposure in responses.\n    """\n    if not ssn or len(ssn) < 4:\n        return "***"\n    \n    return f"XXX-XX-{ssn[-4:]}"\n\n\ndef sanitize_for_output(input_str):\n    """\n    Sanitizes output to prevent injection attacks.\n    SECURITY: Removes potentially dangerous characters.\n    """\n    if not input_str:\n        return ""\n    \n    # Remove or escape potentially dangerous characters\n    dangerous_chars = ['<', '>', '&', '"', "'"]\n    for char in dangerous_chars:\n        input_str = input_str.replace(char, '')\n    \n    return input_str\n\n\nif __name__ == '__main__':\n    # Test cases\n    print("=== Test Cases ===")\n    \n    with app.test_client() as client:\n        # Test case 1: Valid session with data\n        with client.session_transaction() as sess:\n            sess['username'] = 'john_doe'\n            sess['email'] = 'john@example.com'\n            sess['ssn'] = '123456789'\n        response = client.get('/info')\n        print(f"Test 1: {response.data.decode()}")\n        \n        # Test case 2: Another valid session\n        with client.session_transaction() as sess:\n            sess['username'] = 'jane_smith'\n            sess['email'] = 'jane@test.org'\n            sess['ssn'] = '987654321'\n        response = client.get('/info')\n        print(f"Test 2: {response.data.decode()}")\n        \n        # Test case 3: Session without username\n        with client.session_transaction() as sess:\n            sess.clear()\n            sess['email'] = 'test@test.com'\n        response = client.get('/info')\n        print(f"Test 3: {response.data.decode()} (Status: {response.status_code})")\n        \n        # Test case 4: Empty session\n        with client.session_transaction() as sess:\n            sess.clear()\n        response = client.get('/info')\n        print(f"Test 4: {response.data.decode()} (Status: {response.status_code})")\n        \n        # Test case 5: Session with special characters\n        with client.session_transaction() as sess:\n            sess['username'] = '<script>alert("xss")</script>'\n            sess['email'] = 'user@domain.com'\n            sess['ssn'] = '111223333'\n        response = client.get('/info')\n        print(f"Test 5: {response.data.decode()}")\n    \n    print("==================\\n")\n    \n    # Run server in production mode with HTTPS only\n    # SECURITY: Never run with debug=True in production\n    app.run(host='127.0.0.1', port=5000, debug=False, ssl_context='adhoc')
