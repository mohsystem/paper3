
import os
from pathlib import Path

SAFE_DIRECTORY = "/var/safe_files/"

def read_file_from_safe_directory(filename):
    try:
        # Input validation
        if not filename:
            return "Error: Invalid filename"
        
        # Sanitize filename - remove any path separators
        sanitized_filename = filename.replace('/', '').replace('\\\\', '')\n        \n        # Create paths and resolve to canonical paths\n        safe_dir = Path(SAFE_DIRECTORY).resolve()\n        requested_file = (safe_dir / sanitized_filename).resolve()\n        \n        # Security check: ensure the resolved path is within safe directory\n        if not str(requested_file).startswith(str(safe_dir)):\n            return "Error: Access denied - file outside safe directory"\n        \n        # Check if file exists\n        if not requested_file.exists():\n            return "Error: File not found"\n        \n        # Check if it's a regular file
        if not requested_file.is_file():
            return "Error: Not a regular file"
        
        # Read and return file content
        with open(requested_file, 'r', encoding='utf-8') as f:
            return f.read()
            
    except IOError:
        return "Error: Unable to read file"
    except PermissionError:
        return "Error: Access denied"
    except Exception:
        return "Error: An error occurred"

def main():
    # Test cases
    print("Test 1 - Valid file:")
    print(read_file_from_safe_directory("document.txt"))
    
    print("\\nTest 2 - Path traversal attempt:")
    print(read_file_from_safe_directory("../etc/passwd"))
    
    print("\\nTest 3 - Absolute path attempt:")
    print(read_file_from_safe_directory("/etc/passwd"))
    
    print("\\nTest 4 - Empty filename:")
    print(read_file_from_safe_directory(""))
    
    print("\\nTest 5 - Non-existent file:")
    print(read_file_from_safe_directory("nonexistent.txt"))

if __name__ == "__main__":
    main()
