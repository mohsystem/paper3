
from flask import Flask, render_template_string, abort
import re
from html import escape
from functools import wraps

app = Flask(__name__)

# Whitelist pattern for username - only alphanumeric and underscore allowed
# This prevents path traversal, XSS, and other injection attacks
USERNAME_PATTERN = re.compile(r'^[a-zA-Z0-9_]{1,50}$')
MAX_USERNAME_LENGTH = 50

def validate_username(f):
    """Decorator to validate username parameter"""
    @wraps(f)
    def decorated_function(username):
        # Validate username length
        if not username or len(username) > MAX_USERNAME_LENGTH:
            abort(400, description="Invalid username length")
        
        # Validate username against whitelist pattern
        if not USERNAME_PATTERN.match(username):
            abort(400, description="Invalid username format")
        
        return f(username)
    return decorated_function

@app.route('/hello/<username>')
@validate_username
def hello(username):
    """\n    Route handler for /hello/<username>\n    Validates username and renders template with escaped output\n    """
    # Additional HTML escaping for defense in depth
    # Flask's render_template_string auto-escapes by default, but we double-check\n    safe_username = escape(username)\n    \n    # Template content - using Jinja2 auto-escaping\n    # The {{ username }} syntax in Jinja2 automatically escapes HTML\n    template_content = """\n    <!DOCTYPE html>\n    <html>\n    <head>\n        <meta charset="UTF-8">\n        <meta http-equiv="X-Content-Type-Options" content="nosniff">\n        <meta http-equiv="Content-Security-Policy" content="default-src 'self'">\n        <title>Hello</title>\n    </head>\n    <body>\n        <h1>Hello, {{ username }}!</h1>\n        <p>Welcome to the page.</p>\n    </body>\n    </html>\n    """\n    \n    # render_template_string with auto-escaping enabled (default in Flask)\n    return render_template_string(template_content, username=safe_username)\n\n@app.errorhandler(400)\ndef bad_request(e):\n    """Generic error handler - don't leak internal details"""\n    return "Invalid request", 400\n\n@app.errorhandler(404)\ndef not_found(e):\n    """Generic 404 handler"""\n    return "Not Found", 404\n\nif __name__ == '__main__':\n    # Test cases\n    print("Test Case 1 - Valid alphanumeric username:")\n    with app.test_client() as client:\n        response = client.get('/hello/john123')\n        print(f"Status: {response.status_code}")\n        print(response.get_data(as_text=True)[:100] + "...\
")\n    \n    print("Test Case 2 - Valid username with underscore:")\n    with app.test_client() as client:\n        response = client.get('/hello/user_name')\n        print(f"Status: {response.status_code}")\n        print(response.get_data(as_text=True)[:100] + "...\
")\n    \n    print("Test Case 3 - Simple username:")\n    with app.test_client() as client:\n        response = client.get('/hello/alice')\n        print(f"Status: {response.status_code}")\n        print(response.get_data(as_text=True)[:100] + "...\
")\n    \n    print("Test Case 4 - XSS attempt (should be rejected):")\n    with app.test_client() as client:\n        response = client.get('/hello/<script>alert("xss")</script>')\n        print(f"Status: {response.status_code}")\n        print(response.get_data(as_text=True) + "\
")\n    \n    print("Test Case 5 - Path traversal attempt (should be rejected):")\n    with app.test_client() as client:\n        response = client.get('/hello/../etc/passwd')\n        print(f"Status: {response.status_code}")\n        print(response.get_data(as_text=True) + "\
")
    
    # Run the Flask development server
    # In production, use a proper WSGI server like gunicorn with TLS
    app.run(host='127.0.0.1', port=5000, debug=False)
