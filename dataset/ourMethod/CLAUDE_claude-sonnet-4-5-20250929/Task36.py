
import sys
from pathlib import Path
from typing import Optional

# Maximum file size to prevent excessive memory usage: 10MB
MAX_FILE_SIZE = 10 * 1024 * 1024


def validate_file_path(file_path: str) -> bool:
    """\n    Security: Validates file path to prevent directory traversal attacks\n    \n    Args:\n        file_path: The file path to validate\n        \n    Returns:\n        True if valid, False otherwise\n    """
    if not file_path or not isinstance(file_path, str):
        print("Error: File path is None or empty", file=sys.stderr)
        return False
    
    # Security: Reject paths with directory traversal patterns
    if ".." in file_path:
        print("Error: Path traversal not allowed", file=sys.stderr)
        return False
    
    # Security: Reject absolute paths for security
    if file_path.startswith("/") or (len(file_path) >= 2 and file_path[1] == ":"):
        print("Error: Absolute paths not allowed", file=sys.stderr)
        return False
    
    return True


def read_and_print_file(filename: str) -> bool:
    """\n    Security: Reads and prints file contents with validation and size limits\n    \n    Args:\n        filename: The name of the file to read\n        \n    Returns:\n        True on success, False on failure\n    """
    # Security: Validate input path
    if not validate_file_path(filename):
        return False
    
    try:
        # Security: Use Path for safe path operations
        file_path = Path(filename).resolve()
        
        # Security: Get current working directory for validation
        cwd = Path.cwd().resolve()
        
        # Security: Ensure resolved path is under current directory
        try:
            file_path.relative_to(cwd)
        except ValueError:
            print("Error: Path resolves outside working directory", file=sys.stderr)
            return False
        
        # Security: Check if file exists and is a regular file
        if not file_path.exists():
            print(f"Error: File does not exist: {filename}", file=sys.stderr)
            return False
        
        if not file_path.is_file():
            print(f"Error: Not a regular file: {filename}", file=sys.stderr)
            return False
        
        # Security: Check file size before reading
        file_size = file_path.stat().st_size
        if file_size > MAX_FILE_SIZE:
            print("Error: File exceeds maximum size limit", file=sys.stderr)
            return False
        
        # Security: Read file with proper encoding and error handling
        with file_path.open('r', encoding='utf-8', errors='replace') as f:
            content = f.read(MAX_FILE_SIZE)
            
        # Print file contents
        print(content, end='')
        
        return True
        
    except PermissionError as e:
        print(f"Error: Permission denied: {e}", file=sys.stderr)
        return False
    except OSError as e:
        print(f"Error: Cannot read file: {e}", file=sys.stderr)
        return False
    except MemoryError:
        print("Error: File too large to read into memory", file=sys.stderr)
        return False
    except Exception as e:
        print(f"Error: Unexpected error: {e}", file=sys.stderr)
        return False


def main() -> None:
    """Main function with test cases"""
    # Test cases
    test_files = [
        "test1.txt",
        "test2.txt",
        "test3.txt",
        "nonexistent.txt",
        "../etc/passwd"  # Should be rejected
    ]
    
    if len(sys.argv) > 1:
        # User provided filename as argument
        if not read_and_print_file(sys.argv[1]):
            sys.exit(1)
    else:
        # Run test cases
        print("Running test cases:")
        print("===================\\n")
        
        for i, test_file in enumerate(test_files, 1):
            print(f"Test {i}: Reading file '{test_file}'")
            print("-------------------")
            
            if read_and_print_file(test_file):
                print()
            
            print()


if __name__ == "__main__":
    main()
