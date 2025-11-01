
import os
from pathlib import Path
from urllib.parse import unquote
from flask import Flask, request, send_file, abort
import io

app = Flask(__name__)

# Trusted base directory for images - must be absolute
BASE_DIR = Path("images").resolve()
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB limit

# Ensure base directory exists
BASE_DIR.mkdir(parents=True, exist_ok=True)

# Decorator: associates /img URL with img() function
@app.route('/img')
def img_endpoint():
    """HTTP endpoint handler for /img"""
    # Get name parameter from URL query string
    name = request.args.get('name')
    
    if not name:
        abort(400, "Missing name parameter")
    
    try:
        # Call img() function to get file content
        file_content = img(name)
        # Return file content as response
        return send_file(
            io.BytesIO(file_content),
            mimetype='application/octet-stream',
            as_attachment=False
        )
    except (ValueError, PermissionError, OSError) as e:
        abort(400, "Bad request")
    except FileNotFoundError:
        abort(404, "Not found")
    except Exception:
        abort(500, "Internal error")

def img(name):
    """\n    Opens and returns file content from images directory.\n    Implements secure file access with path validation.\n    """
    # Input validation: reject null, empty, or suspicious names
    if not name or not isinstance(name, str):
        raise ValueError("Invalid file name")
    
    # Decode URL-encoded input
    decoded_name = unquote(name)
    
    # Sanitize: reject path traversal attempts and absolute paths
    if '..' in decoded_name or '/' in decoded_name or '\\\\' in decoded_name or decoded_name.startswith('.'):\n        raise PermissionError("Invalid file name: path traversal not allowed")\n    \n    # Construct path relative to base directory\n    requested_path = (BASE_DIR / decoded_name).resolve()\n    \n    # Verify the resolved path is still within base directory\n    if not requested_path.is_relative_to(BASE_DIR):\n        raise PermissionError("Access denied: path outside base directory")\n    \n    # Open file with follow_symlinks=False to prevent symlink attacks\n    # Using os.open for low-level control, then wrapping with fdopen\n    try:\n        # Open with O_RDONLY and O_NOFOLLOW (no symlink following)\n        flags = os.O_RDONLY | os.O_CLOEXEC\n        if hasattr(os, 'O_NOFOLLOW'):\n            flags |= os.O_NOFOLLOW\n        \n        fd = os.open(requested_path, flags)\n    except OSError as e:\n        if e.errno == 2:  # ENOENT\n            raise FileNotFoundError("File not found")\n        elif e.errno == 40:  # ELOOP - too many symlinks\n            raise PermissionError("Symlink not allowed")\n        raise\n    \n    try:\n        # Validate file attributes after opening (TOCTOU prevention)\n        stat_info = os.fstat(fd)\n        \n        # Reject if not a regular file\n        import stat as stat_module\n        if not stat_module.S_ISREG(stat_info.st_mode):\n            raise PermissionError("Not a regular file")\n        \n        # Check file size limit\n        if stat_info.st_size > MAX_FILE_SIZE or stat_info.st_size < 0:\n            raise PermissionError("File size exceeds limit or is invalid")\n        \n        # Read file content using the opened file descriptor\n        with os.fdopen(fd, 'rb') as f:\n            content = f.read(MAX_FILE_SIZE + 1)\n            \n            # Additional runtime check against size limit\n            if len(content) > MAX_FILE_SIZE:\n                raise PermissionError("File size exceeds limit during read")\n            \n            return content\n    except:\n        # Ensure fd is closed if fdopen wasn't successful
        if fd >= 0:
            try:
                os.close(fd)
            except:
                pass
        raise

if __name__ == '__main__':
    # Create test files
    (BASE_DIR / "test1.txt").write_bytes(b"Test file 1")
    (BASE_DIR / "test2.jpg").write_bytes(b"Image data")
    (BASE_DIR / "safe.png").write_bytes(b"Safe image")
    
    # Test case 1: Valid file
    try:
        content = img("test1.txt")
        print(f"Test 1 passed: {content.decode()}")
    except Exception as e:
        print(f"Test 1 failed: {e}")
    
    # Test case 2: Another valid file
    try:
        content = img("test2.jpg")
        print(f"Test 2 passed: {content.decode()}")
    except Exception as e:
        print(f"Test 2 failed: {e}")
    
    # Test case 3: Path traversal attempt (should fail)
    try:
        content = img("../etc/passwd")
        print("Test 3 FAILED: Path traversal succeeded")
    except PermissionError:
        print("Test 3 passed: Path traversal blocked")
    
    # Test case 4: Empty input (should fail)
    try:
        content = img("")
        print("Test 4 FAILED: Empty string accepted")
    except ValueError:
        print("Test 4 passed: Empty string rejected")
    
    # Test case 5: Non-existent file (should fail)
    try:
        content = img("nonexistent.txt")
        print("Test 5 FAILED: Non-existent file returned data")
    except FileNotFoundError:
        print("Test 5 passed: Non-existent file handled")
    
    print("\\nStarting Flask server on port 5000...")
    app.run(host='127.0.0.1', port=5000, debug=False)
