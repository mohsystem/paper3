
import sqlite3
import re
import os
from urllib.parse import parse_qs, urlparse
from http.server import HTTPServer, BaseHTTPRequestHandler
import html

# Database configuration from environment variables
DB_PATH = os.environ.get('DB_PATH', 'subscribers.db')

# Email validation pattern - RFC 5322 simplified
EMAIL_PATTERN = re.compile(r'^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$')

# Maximum email length to prevent DOS
MAX_EMAIL_LENGTH = 254

def is_valid_email(email):
    """\n    Validates email format and length\n    Args:\n        email: Email string to validate\n    Returns:\n        bool: True if valid, False otherwise\n    """
    if not email or not isinstance(email, str):
        return False
    if len(email) > MAX_EMAIL_LENGTH:
        return False
    return EMAIL_PATTERN.match(email) is not None

def initialize_database():
    """\n    Initializes database with subscribers table\n    """
    try:
        # Use context manager for automatic connection closing
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute('''\n                CREATE TABLE IF NOT EXISTS subscribers (\n                    id INTEGER PRIMARY KEY AUTOINCREMENT,\n                    email TEXT UNIQUE NOT NULL\n                )\n            ''')
            conn.commit()
    except sqlite3.Error as e:
        # Log error internally but don't expose details\n        print(f"Database initialization failed", file=__import__('sys').stderr)\n\ndef handle_unsubscribe(email):\n    """\n    Handles unsubscribe request\n    Args:\n        email: Email address to unsubscribe\n    Returns:\n        str: Response message\n    """\n    # Input validation - check for None/empty\n    if email is None or not email.strip():\n        return "Invalid request: email parameter is required"\n    \n    # Trim and normalize input\n    email = email.strip().lower()\n    \n    # Validate email format and length\n    if not is_valid_email(email):\n        return "Invalid email format"\n    \n    try:\n        # Use context manager for automatic resource cleanup\n        with sqlite3.connect(DB_PATH) as conn:\n            cursor = conn.cursor()\n            \n            # First check if email exists - using parameterized query to prevent SQL injection\n            cursor.execute("SELECT COUNT(*) FROM subscribers WHERE email = ?", (email,))\n            count = cursor.fetchone()[0]\n            \n            if count > 0:\n                # Email exists, proceed with deletion using parameterized query\n                cursor.execute("DELETE FROM subscribers WHERE email = ?", (email,))\n                conn.commit()\n                \n                if cursor.rowcount > 0:\n                    return "Successfully unsubscribed"\n                else:\n                    # Unexpected case - log internally\n                    return "Unsubscribe failed"\n            else:\n                # Email not found in database\n                return "Email address is not subscribed"\n                \n    except sqlite3.Error as e:\n        # Log error details internally, return generic message\n        print("Database error occurred", file=__import__('sys').stderr)\n        return "An error occurred processing your request"\n\nclass UnsubscribeHandler(BaseHTTPRequestHandler):\n    """\n    HTTP handler for unsubscribe endpoint\n    """\n    def do_GET(self):\n        """Handle GET requests"""\n        try:\n            # Parse URL and query parameters\n            parsed_url = urlparse(self.path)\n            \n            # Only handle /unsubscribe endpoint\n            if parsed_url.path != '/unsubscribe':\n                self.send_error(404, "Not Found")\n                return\n            \n            # Parse query parameters safely\n            query_params = parse_qs(parsed_url.query)\n            email = query_params.get('email', [None])[0]\n            \n            # Handle unsubscribe\n            response = handle_unsubscribe(email)\n            \n            # Send response\n            self.send_response(200)\n            self.send_header('Content-Type', 'text/plain; charset=utf-8')\n            self.end_headers()\n            self.wfile.write(response.encode('utf-8'))
            
        except Exception as e:
            # Catch any unexpected exceptions
            self.send_error(500, "Internal Server Error")
    
    def log_message(self, format, *args):
        """Override to control logging"""
        pass

def main():
    """Main function with test cases"""
    # Initialize database
    initialize_database()
    
    # Test cases
    print("Test Case 1 - Valid email unsubscribe:")
    print(handle_unsubscribe("test@example.com"))
    
    print("\\nTest Case 2 - Email not in database:")
    print(handle_unsubscribe("notfound@example.com"))
    
    print("\\nTest Case 3 - Invalid email format:")
    print(handle_unsubscribe("invalid-email"))
    
    print("\\nTest Case 4 - None email:")
    print(handle_unsubscribe(None))
    
    print("\\nTest Case 5 - Empty email:")
    print(handle_unsubscribe(""))

if __name__ == "__main__":
    main()
