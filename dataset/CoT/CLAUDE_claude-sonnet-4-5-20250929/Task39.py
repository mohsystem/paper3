
import os
import re
from pathlib import Path

def display_file_contents(filename):
    if not filename or not filename.strip():
        return "Error: Filename cannot be empty"
    
    # Sanitize input - remove dangerous characters
    filename = re.sub(r'[;&|`$(){}\\[\\]<>\\'\\"]', '', filename)
    
    try:
        # Use Path for secure file handling
        file_path = Path(filename).resolve()
        
        # Prevent path traversal
        if '..' in filename:
            return "Error: Path traversal detected"
        
        # Check if file exists
        if not file_path.exists():
            return "Error: File does not exist"
        
        # Check if it's a file (not directory)\n        if not file_path.is_file():\n            return "Error: Not a valid file"\n        \n        # Check if file is readable\n        if not os.access(file_path, os.R_OK):\n            return "Error: File is not readable"\n        \n        # Read file securely (no system command)\n        content = f"Contents of {filename}:\\n"\n        content += "-----------------------------------\\n"\n        \n        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content += f.read()
        
        return content
        
    except PermissionError:
        return "Error: Permission denied"
    except Exception as e:
        return f"Error: {str(e)}"

def main():
    # Test case 1: Valid text file
    print("Test 1:")
    print(display_file_contents("test1.txt"))
    
    # Test case 2: Empty filename
    print("\\nTest 2:")
    print(display_file_contents(""))
    
    # Test case 3: Path traversal attempt
    print("\\nTest 3:")
    print(display_file_contents("../../../etc/passwd"))
    
    # Test case 4: Command injection attempt
    print("\\nTest 4:")
    print(display_file_contents("test.txt; rm -rf /"))
    
    # Test case 5: Non-existent file
    print("\\nTest 5:")
    print(display_file_contents("nonexistent.txt"))

if __name__ == "__main__":
    main()
