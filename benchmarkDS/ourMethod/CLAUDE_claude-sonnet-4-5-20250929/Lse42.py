
import secrets
import os
from flask import Flask, session, jsonify, abort
from functools import wraps

app = Flask(__name__)

# Rules#3: Never hard code passwords or secrets anywhere in code
# Read secret key from environment variable or generate a secure one
app.config['SECRET_KEY'] = os.environ.get('FLASK_SECRET_KEY') or secrets.token_hex(32)

# Rules#5: Use modern, vetted libraries with proper session configuration
app.config['SESSION_COOKIE_SECURE'] = True  # Only send cookie over HTTPS
app.config['SESSION_COOKIE_HTTPONLY'] = True  # Prevent JavaScript access
app.config['SESSION_COOKIE_SAMESITE'] = 'Strict'  # CSRF protection
app.config['PERMANENT_SESSION_LIFETIME'] = 1800  # 30 minutes timeout

def validate_session_data(f):
    """\n    Rules#2: Input validation decorator\n    Validates that session data exists and is properly formatted\n    """
    @wraps(f)
    def decorated_function(*args, **kwargs):
        # Rules#1: Fail closed with clear, minimal error messages
        if 'username' not in session or 'email' not in session or 'ssn' not in session:
            abort(401, description="Unauthorized access")
        
        # Rules#2: Check type and length for session data
        username = session.get('username', '')
        email = session.get('email', '')
        ssn = session.get('ssn', '')
        
        if not isinstance(username, str) or len(username) > 100:
            abort(400, description="Invalid session data")
        if not isinstance(email, str) or len(email) > 255:
            abort(400, description="Invalid session data")
        if not isinstance(ssn, str) or len(ssn) > 20:
            abort(400, description="Invalid session data")
            
        return f(*args, **kwargs)
    return decorated_function

@app.route('/info', methods=['GET'])
@validate_session_data
def info():
    """\n    Rules#10: Never log secrets or sensitive data\n    Rules#11: Do not leak internal details in responses\n    Retrieves user information from session\n    """
    try:
        # Retrieve validated session data
        username = session.get('username', '')
        email = session.get('email', '')
        ssn = session.get('ssn', '')
        
        # Rules#9: Handle sensitive data carefully
        # Mask SSN for display (show only last 4 digits)
        masked_ssn = 'XXX-XX-' + ssn[-4:] if len(ssn) >= 4 else '***'
        
        # Return as structured JSON instead of plain string
        # This prevents injection and provides better API design
        return jsonify({
            'username': username,
            'email': email,
            'ssn': masked_ssn
        }), 200
        
    except Exception as e:
        # Rules#11: Do not leak internal details to callers
        # Rules#10: Log errors securely without sensitive data
        app.logger.error("Error retrieving user info", exc_info=True)
        abort(500, description="Internal server error")

@app.errorhandler(400)
@app.errorhandler(401)
@app.errorhandler(500)
def handle_error(error):
    """\n    Rules#11: Return generic messages to callers\n    Centralized error handling\n    """
    return jsonify({'error': error.description}), error.code

def main():
    """\n    Main function with test cases\n    Note: In production, sessions would be set via login endpoint\n    """
    # Test case setup (for demonstration only)
    with app.test_client() as client:
        with client.session_transaction() as test_session:
            # Test Case 1: Valid session data
            test_session['username'] = 'john_doe'
            test_session['email'] = 'john@example.com'
            test_session['ssn'] = '123-45-6789'
        
        print("Test 1 - Valid session:")
        response = client.get('/info')
        print(f"Status: {response.status_code}, Data: {response.get_json()}\\n")
        
        # Test Case 2: Missing username
        with client.session_transaction() as test_session:
            test_session.clear()
            test_session['email'] = 'jane@example.com'
            test_session['ssn'] = '987-65-4321'
        
        print("Test 2 - Missing username:")
        response = client.get('/info')
        print(f"Status: {response.status_code}, Data: {response.get_json()}\\n")
        
        # Test Case 3: Missing email
        with client.session_transaction() as test_session:
            test_session.clear()
            test_session['username'] = 'alice_smith'
            test_session['ssn'] = '555-55-5555'
        
        print("Test 3 - Missing email:")
        response = client.get('/info')
        print(f"Status: {response.status_code}, Data: {response.get_json()}\\n")
        
        # Test Case 4: All fields present with different data
        with client.session_transaction() as test_session:
            test_session.clear()
            test_session['username'] = 'bob_jones'
            test_session['email'] = 'bob@example.com'
            test_session['ssn'] = '111-22-3333'
        
        print("Test 4 - Valid session with different user:")
        response = client.get('/info')
        print(f"Status: {response.status_code}, Data: {response.get_json()}\\n")
        
        # Test Case 5: Empty session (no data)
        with client.session_transaction() as test_session:
            test_session.clear()
        
        print("Test 5 - Empty session:")
        response = client.get('/info')
        print(f"Status: {response.status_code}, Data: {response.get_json()}\\n")

if __name__ == '__main__':
    main()
