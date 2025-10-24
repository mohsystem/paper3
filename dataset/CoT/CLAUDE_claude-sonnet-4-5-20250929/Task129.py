
import os
import re
from pathlib import Path

SAFE_DIRECTORY = "safe_files"
ALLOWED_EXTENSIONS = {".txt", ".log", ".csv", ".json", ".xml"}

def retrieve_file(file_name):
    """\n    Securely retrieve file content from the safe directory.\n    \n    Args:\n        file_name: Name of the file to retrieve\n        \n    Returns:\n        String containing file content or error message\n    """
    try:
        # Input validation
        if not file_name or not file_name.strip():
            return "Error: File name cannot be empty"
        
        # Remove path traversal attempts
        file_name = file_name.replace("..", "").replace("/", "").replace("\\\\", "")\n        \n        # Validate file extension\n        if not is_allowed_extension(file_name):\n            return "Error: File extension not allowed"\n        \n        # Create safe directory if it doesn't exist\n        safe_dir = Path(SAFE_DIRECTORY)\n        safe_dir.mkdir(exist_ok=True)\n        \n        # Build safe file path\n        base_path = safe_dir.resolve()\n        file_path = (base_path / file_name).resolve()\n        \n        # Verify the file is within the safe directory (prevent path traversal)\n        if not str(file_path).startswith(str(base_path)):\n            return "Error: Access denied - path traversal detected"\n        \n        # Check if file exists\n        if not file_path.is_file():\n            return "Error: File not found"\n        \n        # Check file size (10MB limit)\n        if file_path.stat().st_size > 10 * 1024 * 1024:\n            return "Error: File too large"\n        \n        # Read file content\n        with open(file_path, 'r', encoding='utf-8') as f:\n            content = f.read()\n        \n        return f"Success: File retrieved\
{content}"\n        \n    except Exception as e:\n        return f"Error: {str(e)}"\n\ndef is_allowed_extension(file_name):\n    """Check if file has an allowed extension."""\n    return any(file_name.lower().endswith(ext) for ext in ALLOWED_EXTENSIONS)\n\ndef setup_test_files():\n    """Create test files for demonstration."""\n    try:\n        safe_dir = Path(SAFE_DIRECTORY)\n        safe_dir.mkdir(exist_ok=True)\n        \n        test_file = safe_dir / "test1.txt"\n        with open(test_file, 'w') as f:\n            f.write("This is a test file content.\
Secure file retrieval system.")\n    except Exception as e:\n        print(f"Setup error: {e}")\n\nif __name__ == "__main__":\n    # Create test files\n    setup_test_files()\n    \n    # Test Case 1: Valid file retrieval\n    print("Test 1 - Valid file:")\n    print(retrieve_file("test1.txt"))\n    \n    # Test Case 2: Path traversal attempt\n    print("\
Test 2 - Path traversal attempt:")\n    print(retrieve_file("../../../etc/passwd"))\n    \n    # Test Case 3: Invalid extension\n    print("\
Test 3 - Invalid extension:")\n    print(retrieve_file("malicious.exe"))\n    \n    # Test Case 4: Empty filename\n    print("\
Test 4 - Empty filename:")\n    print(retrieve_file(""))\n    \n    # Test Case 5: Non-existent file\n    print("\
Test 5 - Non-existent file:")\n    print(retrieve_file("nonexistent.txt"))
