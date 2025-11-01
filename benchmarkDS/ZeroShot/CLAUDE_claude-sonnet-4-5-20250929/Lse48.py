
import os
import re
from pathlib import Path

ALLOWED_DIRECTORY = "./images/"

def img(file_param):
    """\n    Secure implementation of image file reading with path traversal protection\n    """
    if not file_param:
        print("File parameter is required", file=sys.stderr)
        return None
    
    # Secure: Sanitize input to prevent path traversal
    sanitized_file = re.sub(r'[^a-zA-Z0-9._-]', '', file_param)
    
    # Prevent path traversal attempts
    if '..' in sanitized_file or '/' in sanitized_file or '\\\\' in sanitized_file:\n        print("Invalid file name", file=sys.stderr)\n        return None\n    \n    try:\n        # Construct the full path\n        allowed_path = Path(ALLOWED_DIRECTORY).resolve()\n        file_path = (allowed_path / sanitized_file).resolve()\n        \n        # Ensure the file is within allowed directory\n        if not str(file_path).startswith(str(allowed_path)):\n            print("Access denied: Path traversal attempt detected", file=sys.stderr)\n            return None\n        \n        # Check if file exists and is readable\n        if not file_path.exists() or not file_path.is_file():\n            print("File not found or not readable", file=sys.stderr)\n            return None\n        \n        # Read file in binary mode\n        with open(file_path, 'rb') as f:\n            return f.read()\n    except Exception as e:\n        print(f"Error reading file: {e}", file=sys.stderr)\n        return None\n\nif __name__ == "__main__":\n    import sys\n    \n    # Create test directory and files\n    os.makedirs(ALLOWED_DIRECTORY, exist_ok=True)\n    \n    try:\n        with open(os.path.join(ALLOWED_DIRECTORY, "test1.txt"), 'wb') as f:\n            f.write(b"Test content 1")\n        with open(os.path.join(ALLOWED_DIRECTORY, "test2.jpg"), 'wb') as f:
            f.write(b"Image content")
    except Exception as e:
        print(f"Setup error: {e}", file=sys.stderr)
    
    # Test case 1: Valid file
    print("Test 1 - Valid file:")
    result1 = img("test1.txt")
    print("Success:", result1.decode() if result1 else "Failed")
    
    # Test case 2: Another valid file
    print("\\nTest 2 - Valid image file:")
    result2 = img("test2.jpg")
    print("Success:", result2.decode() if result2 else "Failed")
    
    # Test case 3: Path traversal attempt (should be blocked)
    print("\\nTest 3 - Path traversal attempt:")
    result3 = img("../etc/passwd")
    print("Blocked (Expected)" if not result3 else "Success")
    
    # Test case 4: None parameter
    print("\\nTest 4 - None parameter:")
    result4 = img(None)
    print("Blocked (Expected)" if not result4 else "Success")
    
    # Test case 5: Non-existent file
    print("\\nTest 5 - Non-existent file:")
    result5 = img("nonexistent.txt")
    print("Failed (Expected)" if not result5 else "Success")
