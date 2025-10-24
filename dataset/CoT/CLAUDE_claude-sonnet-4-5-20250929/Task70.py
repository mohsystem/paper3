
import os
import sys
from pathlib import Path

MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB limit

def read_file_content(file_path):
    """\n    Securely read and return the content of a file.\n    \n    Args:\n        file_path: Path to the file to read\n        \n    Returns:\n        String containing file content or error message\n    """
    if not file_path or not isinstance(file_path, str) or not file_path.strip():
        return "Error: File path cannot be null or empty"
    
    try:
        # Normalize and resolve the path to prevent path traversal
        path = Path(file_path).resolve()
        
        # Check if file exists
        if not path.exists():
            return f"Error: File does not exist: {file_path}"
        
        # Check if it's a regular file (not a directory)\n        if not path.is_file():\n            return f"Error: Path is not a regular file: {file_path}"\n        \n        # Check file size to prevent memory exhaustion\n        file_size = path.stat().st_size\n        if file_size > MAX_FILE_SIZE:\n            return "Error: File size exceeds maximum allowed size (10MB)"\n        \n        # Read file content securely with UTF-8 encoding\n        with open(path, 'r', encoding='utf-8', errors='replace') as file:\n            content = file.read()\n            return content\n            \n    except PermissionError:\n        return f"Error: Permission denied to read file: {file_path}"\n    except OSError as e:\n        return f"Error: Unable to read file - {str(e)}"\n    except Exception as e:\n        return f"Error: Unexpected error - {str(e)}"\n\ndef main():\n    if len(sys.argv) < 2:\n        print("Usage: python Task70.py <file_path>")\n        print("\\nRunning test cases...\\n")\n        \n        # Test case 1: Valid file\n        import tempfile\n        try:\n            with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.txt') as f:\n                test_file1 = f.name\n                f.write("Hello, World!")\n            print("Test 1 - Valid file:")\n            print(read_file_content(test_file1))\n            os.unlink(test_file1)\n        except Exception as e:\n            print(f"Test 1 failed: {str(e)}")\n        \n        # Test case 2: Non-existent file\n        print("\\nTest 2 - Non-existent file:")\n        print(read_file_content("/nonexistent/file.txt"))\n        \n        # Test case 3: None path\n        print("\\nTest 3 - None path:")\n        print(read_file_content(None))\n        \n        # Test case 4: Empty path\n        print("\\nTest 4 - Empty path:")\n        print(read_file_content(""))\n        \n        # Test case 5: File with special characters\n        try:\n            with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.txt') as f:
                test_file5 = f.name
                f.write("Content with special chars: @#$%^&*()")
            print("\\nTest 5 - File with special characters:")
            print(read_file_content(test_file5))
            os.unlink(test_file5)
        except Exception as e:
            print(f"Test 5 failed: {str(e)}")
    else:
        file_path = sys.argv[1]
        content = read_file_content(file_path)
        print(content)

if __name__ == "__main__":
    main()
