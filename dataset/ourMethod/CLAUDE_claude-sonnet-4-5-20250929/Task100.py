
import re
import sys
from pathlib import Path
from typing import Optional

MAX_FILE_SIZE = 10_000_000  # 10MB limit
BASE_DIR = Path.cwd().resolve()


def search_file_with_pattern(pattern_str: str, file_name: str) -> str:
    """\n    Search a file using a regular expression pattern.\n    \n    Args:\n        pattern_str: Regular expression pattern string\n        file_name: Name of the file to search\n        \n    Returns:\n        String containing search results or error message\n    """
    if not pattern_str:
        return "Error: Pattern cannot be empty"
    if not file_name:
        return "Error: File name cannot be empty"
    
    # Validate and sanitize file path
    try:
        base_path = BASE_DIR
        file_path = (base_path / file_name).resolve()
        
        # Ensure the resolved path is within base directory
        if not str(file_path).startswith(str(base_path)):
            return "Error: Access denied - path outside allowed directory"
        
        # Check if it's a regular file\n        if not file_path.is_file():\n            return "Error: Not a regular file"\n        \n        # Check file size\n        file_size = file_path.stat().st_size\n        if file_size > MAX_FILE_SIZE:\n            return "Error: File too large (max 10MB)"\n            \n    except (OSError, ValueError) as e:\n        return f"Error: Cannot access file - {str(e)}"\n    except Exception as e:\n        return f"Error: Invalid file path - {str(e)}"\n    \n    # Compile pattern\n    try:\n        pattern = re.compile(pattern_str)\n    except re.error as e:\n        return f"Error: Invalid regex pattern - {str(e)}"\n    \n    # Search file content\n    result = []\n    match_count = 0\n    \n    try:\n        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:\n            for line_number, line in enumerate(f, 1):\n                line = line.rstrip('\
\\r')
                if pattern.search(line):
                    match_count += 1
                    result.append(f"Line {line_number}: {line}")
    except OSError as e:
        return f"Error: Failed to read file - {str(e)}"
    
    if match_count == 0:
        return "No matches found"
    
    return f"Found {match_count} match(es):\\n" + "\\n".join(result)


def main():
    if len(sys.argv) == 3:
        pattern = sys.argv[1]
        file_name = sys.argv[2]
        print(search_file_with_pattern(pattern, file_name))
    else:
        # Test cases
        print("=== Test Case 1: Simple pattern ===")
        print(search_file_with_pattern("test", "testfile1.txt"))
        
        print("\\n=== Test Case 2: Complex pattern ===")
        print(search_file_with_pattern(r"[0-9]{3}", "testfile2.txt"))
        
        print("\\n=== Test Case 3: Invalid pattern ===")
        print(search_file_with_pattern("[invalid", "testfile1.txt"))
        
        print("\\n=== Test Case 4: Non-existent file ===")
        print(search_file_with_pattern("test", "nonexistent.txt"))
        
        print("\\n=== Test Case 5: Path traversal attempt ===")
        print(search_file_with_pattern("test", "../../../etc/passwd"))


if __name__ == "__main__":
    main()
