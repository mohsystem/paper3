
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs, unquote
import json
import sys

# Security: Maximum payload size to prevent DoS attacks
MAX_PAYLOAD_SIZE = 10240  # 10KB limit

def process_payload(payload):
    """\n    Process the payload parameter and extract the name value.\n    \n    Security measures:\n    - Input validation for size and type\n    - Safe JSON parsing without code execution\n    - Output sanitization\n    """
    # Security: Validate input is not None and within size limits
    if payload is None or len(payload) == 0:
        return json.dumps({"error": "Invalid input"})
    
    if len(payload) > MAX_PAYLOAD_SIZE:
        return json.dumps({"error": "Payload too large"})
    
    try:
        # Security: Use json.loads which is safe and doesn't execute code\n        # Never use eval() or exec() on untrusted input\n        payload_dict = json.loads(payload)\n        \n        # Security: Validate that payload_dict is actually a dictionary\n        if not isinstance(payload_dict, dict):\n            return json.dumps({"error": "Invalid payload format"})\n        \n        # Security: Check if 'name' key exists before accessing\n        if "name" not in payload_dict:\n            return json.dumps({"error": "Missing name field"})\n        \n        # Security: Get the name value, treating as untrusted data\n        name = payload_dict.get("name")\n        \n        # Security: Validate name is a string\n        if not isinstance(name, str):\n            return json.dumps({"error": "Invalid name type"})\n        \n        # Security: Use json.dumps for safe output serialization\n        # This automatically escapes special characters\n        return json.dumps({"name": name})\n        \n    except json.JSONDecodeError:\n        # Security: Generic error message, no internal details leaked\n        return json.dumps({"error": "Invalid JSON format"})\n    except Exception:\n        # Security: Catch-all for unexpected errors, no details leaked\n        return json.dumps({"error": "Processing error"})\n\n\nclass ApiHandler(BaseHTTPRequestHandler):\n    """HTTP request handler for /api endpoint"""\n    \n    def do_GET(self):\n        """Handle GET requests to /api endpoint"""\n        # Security: Parse URL safely\n        parsed_path = urlparse(self.path)\n        \n        # Security: Check if path matches /api\n        if parsed_path.path != "/api":\n            self.send_error(404)\n            return\n        \n        # Security: Parse query parameters safely with explicit encoding\n        query_params = parse_qs(parsed_path.query)\n        \n        # Security: Get payload parameter, default to None if not present\n        payload = query_params.get("payload", [None])[0]\n        \n        # Process the payload\n        result = process_payload(payload)\n        \n        # Security: Set secure response headers\n        self.send_response(200)\n        self.send_header("Content-Type", "application/json")\n        self.send_header("X-Content-Type-Options", "nosniff")\n        self.end_headers()\n        \n        # Security: Encode response as UTF-8\n        self.wfile.write(result.encode("utf-8"))\n    \n    def log_message(self, format, *args):\n        """Override to prevent logging sensitive data"""\n        # Security: Minimal logging, no user data exposed\n        pass\n\n\ndef main():\n    """Main function to start the web server and run test cases"""\n    # Create and start HTTP server on port 8000\n    server_address = ("", 8000)\n    httpd = HTTPServer(server_address, ApiHandler)\n    \n    print("Server started on port 8000")\n    print("\\nTest cases:")\n    print("1. Valid JSON:", process_payload('{"name":"John"}'))\n    print("2. Valid JSON with special chars:", process_payload('{"name":"O\\'Brien"}'))\n    print("3. Missing name field:", process_payload('{"age":30}'))\n    print("4. Invalid JSON:", process_payload('{invalid}'))
    print("5. None payload:", process_payload(None))
    
    # Start server (this will run indefinitely)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\\nServer stopped")
        httpd.shutdown()


if __name__ == "__main__":
    main()
