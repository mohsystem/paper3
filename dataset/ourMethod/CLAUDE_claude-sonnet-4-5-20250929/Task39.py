
import sys
import os
from pathlib import Path
from typing import Optional


def is_safe_filename(filename: str) -> bool:
    """Security: Validate filename to prevent command injection and path traversal."""
    if not filename or len(filename) > 255:
        return False
    
    # Reject directory traversal attempts
    if ".." in filename:
        return False
    
    # Reject absolute paths
    if filename.startswith(("/", "\\\\")):\n        return False\n    \n    # Allow only alphanumeric, underscore, hyphen, and dot\n    for c in filename:\n        if not (c.isalnum() or c in ("_", "-", ".")):\n            return False\n    \n    # Reject filenames starting with dot (hidden files)\n    if filename.startswith("."):\n        return False\n    \n    return True\n\n\ndef display_file_contents(filename: str) -> None:\n    """Security: Safe file content display without system command."""\n    # Security: Validate filename before use\n    if not is_safe_filename(filename):\n        print("Error: Invalid filename format", file=sys.stderr)\n        return\n    \n    try:\n        # Security: Resolve path and ensure it's in current directory\n        file_path = Path(filename).resolve()\n        current_dir = Path.cwd().resolve()\n        \n        # Security: Verify path is within current directory\n        if not str(file_path).startswith(str(current_dir)):\n            print("Error: File must be in current directory", file=sys.stderr)\n            return\n        \n        # Security: Open file safely - no os.system() to prevent command injection\n        # Use context manager for automatic resource cleanup\n        with open(file_path, 'r', encoding='utf-8') as file:\n            print(f"Contents of '{filename}':")\n            print("-" * 40)\n            \n            # Security: Read file safely\n            contents = file.read()\n            print(contents, end='')\n            \n            print("\
" + "-" * 40)\n            \n    except FileNotFoundError:\n        print(f"Error: Cannot open file '{filename}'", file=sys.stderr)\n    except PermissionError:\n        print(f"Error: Permission denied for file '{filename}'", file=sys.stderr)\n    except IOError as e:\n        print(f"Error: Cannot read file '{filename}': {e}", file=sys.stderr)\n\n\ndef main() -> None:\n    """Main function with test cases."""\n    try:\n        # Security: Use input() which is safe in Python 3\n        filename = input("Enter filename to display: ").strip()\n        \n        print("\
=== Test Case 1: User Input ===")\n        display_file_contents(filename)\n        \n    except EOFError:\n        print("Error: Failed to read input", file=sys.stderr)\n        return\n    except KeyboardInterrupt:\n        print("\
Interrupted by user", file=sys.stderr)\n        return\n    \n    # Create test files for automated testing\n    try:\n        with open("test1.txt", "w", encoding="utf-8") as f:\n            f.write("This is test file 1.\
Line 2.\
")\n        \n        with open("test2.txt", "w", encoding="utf-8") as f:\n            f.write("Test file 2 contents.\
")\n        \n        with open("test3.txt", "w", encoding="utf-8") as f:\n            f.write("Another test file.\
Multiple lines.\
Third line.\
")\n    except IOError:\n        print("Warning: Could not create test files", file=sys.stderr)\n    \n    print("\
=== Test Case 2: Valid filename ===")\n    display_file_contents("test1.txt")\n    \n    print("\
=== Test Case 3: Another valid filename ===")\n    display_file_contents("test2.txt")\n    \n    print("\
=== Test Case 4: Invalid filename (path traversal) ===")\n    display_file_contents("../etc/passwd")\n    \n    print("\
=== Test Case 5: Invalid filename (special chars) ===")\n    display_file_contents("file;rm -rf")\n    \n    print("\
=== Test Case 6: Non-existent file ===")\n    display_file_contents("nonexistent.txt")\n\n\nif __name__ == "__main__":
    main()
