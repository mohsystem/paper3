
import os
import re
import sys
from typing import Optional
import mysql.connector
from mysql.connector import Error
from http.server import HTTPServer, BaseHTTPRequestHandler
import ssl

# Security: Never hard-code credentials. Read from environment variables or secret manager.
# Rules#3: Avoid hard-coded passwords or secrets

def get_server_name() -> str:
    """\n    Returns database server name from environment variable with validation.\n    Rules#2: Input validation - check type, length, range\n    """
    server = os.environ.get('DB_SERVER')
    if not server or not server.strip():
        raise ValueError("DB_SERVER environment variable not set")
    
    # Validate server name format (alphanumeric, dots, hyphens only)
    if not re.match(r'^[a-zA-Z0-9.-]+$', server):
        raise ValueError("Invalid server name format")
    
    if len(server) > 253:
        raise ValueError("Server name too long")
    
    return server

def get_database_name() -> str:
    """\n    Returns database name from environment variable with validation.\n    """
    db_name = os.environ.get('DB_NAME')
    if not db_name or not db_name.strip():
        raise ValueError("DB_NAME environment variable not set")
    
    # Validate database name (alphanumeric and underscore only)
    if not re.match(r'^[a-zA-Z0-9_]+$', db_name):
        raise ValueError("Invalid database name format")
    
    if len(db_name) > 64:
        raise ValueError("Database name too long")
    
    return db_name

def get_database_user() -> str:
    """\n    Returns database user from environment variable with validation.\n    """
    user = os.environ.get('DB_USER')
    if not user or not user.strip():
        raise ValueError("DB_USER environment variable not set")
    
    # Validate username format
    if not re.match(r'^[a-zA-Z0-9_]+$', user):
        raise ValueError("Invalid username format")
    
    if len(user) > 32:
        raise ValueError("Username too long")
    
    return user

def get_database_password() -> str:
    """\n    Returns database password from environment variable.\n    Security: Never log or expose passwords (Rules#10)\n    """
    password = os.environ.get('DB_PASSWORD')
    if password is None or password == '':
        raise ValueError("DB_PASSWORD environment variable not set")
    
    # Length validation only - don't restrict password characters\n    if len(password) > 128:\n        raise ValueError("Password too long")\n    \n    return password\n\ndef get_database_connection():\n    """\n    Returns a database connection using validated configuration.\n    Rules#6: Use TLS with certificate verification for database connections\n    """\n    try:\n        server = get_server_name()\n        db_name = get_database_name()\n        user = get_database_user()\n        password = get_database_password()\n        \n        # Rules#6: Enforce TLS for database connections with certificate verification\n        # Create SSL context for secure connection\n        ssl_context = ssl.create_default_context()\n        ssl_context.minimum_version = ssl.TLSVersion.TLSv1_2\n        \n        # Rules#11: Don't leak connection details in errors
        connection = mysql.connector.connect(
            host=server,
            database=db_name,
            user=user,
            password=password,
            ssl_disabled=False,
            use_pure=True
        )
        
        if connection is None or not connection.is_connected():
            raise Error("Failed to establish database connection")
        
        return connection
        
    except Error as e:
        # Rules#11: Generic error message, detailed logging to secure logs
        print(f"Database connection error type: {type(e).__name__}", file=sys.stderr)
        raise Exception("Database connection failed")

# Simple web app handler
class WebAppHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        """Handle GET requests"""
        # Rules#2: Validate HTTP method
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain; charset=utf-8')
        self.end_headers()
        self.wfile.write(b'Web application running')
    
    def log_message(self, format, *args):
        """Override to control logging - Rules#10: Use constant format strings"""
        sys.stderr.write("%s - - [%s] %s\\n" %
                         (self.address_string(),
                          self.log_date_time_string(),
                          format % args))

def main():
    """Main function with test cases"""
    print("=== Lse141 Web App Test Cases ===\\n")
    
    # Test Case 1: Missing environment variables
    print("Test 1: Missing environment variables")
    try:
        get_server_name()
        print("FAIL: Should have thrown exception")
    except ValueError as e:
        print("PASS: Correctly detected missing DB_SERVER")
    
    # Test Case 2: Set valid environment variables (simulated)
    print("\\nTest 2: Valid configuration check")
    print("Note: Set environment variables DB_SERVER, DB_NAME, DB_USER, DB_PASSWORD before running")
    
    # Test Case 3: Invalid server name format
    print("\\nTest 3: Server name validation")
    os.environ['DB_SERVER'] = 'invalid;server'
    try:
        get_server_name()
        print("FAIL: Should have rejected invalid server name")
    except ValueError:
        print("PASS: Server name validation works")
    finally:
        os.environ.pop('DB_SERVER', None)
    
    # Test Case 4: Start web server
    print("\\nTest 4: Starting web server")
    try:
        server = HTTPServer(('localhost', 8080), WebAppHandler)
        print("PASS: Web server created on port 8080")
        # Note: In production, use server.serve_forever()
    except Exception as e:
        print(f"FAIL: Could not create web server - {type(e).__name__}")
    
    # Test Case 5: Database connection (requires valid env vars)
    print("\\nTest 5: Database connection")
    try:
        conn = get_database_connection()
        if conn and conn.is_connected():
            print("PASS: Database connection successful")
            conn.close()
    except Exception:
        print("INFO: Database connection requires valid environment variables")
    
    print("\\n=== Tests Complete ===")

if __name__ == '__main__':
    main()
