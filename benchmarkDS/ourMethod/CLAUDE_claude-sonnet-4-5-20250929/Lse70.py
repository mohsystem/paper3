
import os
import secrets
import tempfile
from pathlib import Path
from http.server import BaseHTTPRequestHandler, HTTPServer
import threading

# Define upload directory with absolute path and normalization
UPLOAD_DIR = Path("uploads").resolve()
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB max file size
MAX_FILENAME_LENGTH = 255


def upload_file(file_content: bytes, content_type: str) -> str:
    """\n    Securely upload a file to the uploads directory.\n    \n    Args:\n        file_content: The binary content of the file\n        content_type: The content type of the upload\n        \n    Returns:\n        Status message indicating success or failure\n    """
    # Validate content type
    if not content_type or not content_type.startswith("multipart/form-data"):
        return "Error: Invalid content type"
    
    # Enforce maximum file size to prevent DoS attacks
    if len(file_content) > MAX_FILE_SIZE:
        return "Error: File too large"
    
    try:
        # Create uploads directory if it doesn't exist with restrictive permissions\n        UPLOAD_DIR.mkdir(mode=0o700, parents=True, exist_ok=True)\n        \n        # Generate cryptographically secure random filename to prevent overwrites\n        # and path traversal attacks\n        safe_filename = secrets.token_hex(16) + ".bin"\n        \n        # Validate filename length\n        if len(safe_filename) > MAX_FILENAME_LENGTH:\n            return "Error: Filename too long"\n        \n        # Construct target path and normalize it\n        target_path = (UPLOAD_DIR / safe_filename).resolve()\n        \n        # Security check: ensure resolved path is within UPLOAD_DIR\n        # This prevents directory traversal attacks\n        if not str(target_path).startswith(str(UPLOAD_DIR)):\n            return "Error: Invalid file path"\n        \n        # Create a temporary file in the same directory with restrictive permissions\n        # Use os.open with O_CREAT|O_EXCL to prevent race conditions\n        fd = os.open(\n            target_path,\n            os.O_WRONLY | os.O_CREAT | os.O_EXCL,\n            mode=0o600  # Owner read/write only\n        )\n        \n        try:\n            # Write content to file using the opened file descriptor\n            # This avoids TOCTOU vulnerabilities\n            with os.fdopen(fd, 'wb') as f:\n                f.write(file_content)\n                f.flush()\n                os.fsync(f.fileno())  # Ensure data is written to disk\n            \n            return f"File uploaded successfully: {safe_filename}"\n            \n        except Exception as e:\n            # Clean up on error\n            try:\n                os.unlink(target_path)\n            except OSError:\n                pass\n            return "Error: Upload failed"\n            \n    except FileExistsError:\n        return "Error: File already exists"\n    except Exception as e:\n        return "Error: Upload failed"\n\n\nclass UploadHandler(BaseHTTPRequestHandler):\n    """HTTP request handler for file uploads"""\n    \n    def do_POST(self):\n        """Handle POST requests for file upload"""\n        if self.path == "/upload":\n            content_type = self.headers.get("Content-Type", "")\n            content_length = int(self.headers.get("Content-Length", 0))\n            \n            # Enforce size limit at read time to prevent memory exhaustion\n            if content_length > MAX_FILE_SIZE:\n                self.send_response(400)\n                self.end_headers()\n                self.wfile.write(b"Error: File too large")\n                return\n            \n            # Read file content with size limit\n            file_content = self.rfile.read(content_length)\n            \n            # Upload file\n            result = upload_file(file_content, content_type)\n            \n            # Send response\n            self.send_response(200)\n            self.send_header("Content-Type", "text/plain")\n            self.end_headers()\n            self.wfile.write(result.encode('utf-8'))\n        else:\n            self.send_response(404)\n            self.end_headers()\n    \n    def log_message(self, format, *args):\n        """Override to prevent logging sensitive information"""\n        pass\n\n\ndef run_server():\n    """Start the HTTP server"""\n    server = HTTPServer(('localhost', 8000), UploadHandler)
    print("Server started on port 8000")
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()
    return server


def run_test_cases():
    """Run test cases for the upload_file function"""
    print("\\n=== Running Test Cases ===")
    
    # Test case 1: Valid small file upload
    result1 = upload_file(b"Test content 1", "multipart/form-data")
    print(f"Test 1 (valid upload): {result1}")
    
    # Test case 2: Another valid upload
    result2 = upload_file(b"Test content 2", "multipart/form-data")
    print(f"Test 2 (valid upload): {result2}")
    
    # Test case 3: Invalid content type
    result3 = upload_file(b"Test content 3", "text/plain")
    print(f"Test 3 (invalid content type): {result3}")
    
    # Test case 4: Large file (1KB)
    result4 = upload_file(b"x" * 1024, "multipart/form-data")
    print(f"Test 4 (1KB file): {result4}")
    
    # Test case 5: Empty file
    result5 = upload_file(b"", "multipart/form-data")
    print(f"Test 5 (empty file): {result5}")


if __name__ == "__main__":
    server = run_server()
    run_test_cases()
    
    try:
        # Keep server running
        input("\\nPress Enter to stop server...\\n")
    except KeyboardInterrupt:
        pass
    finally:
        server.shutdown()
