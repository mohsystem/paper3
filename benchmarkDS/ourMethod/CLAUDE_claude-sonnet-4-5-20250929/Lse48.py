
import os
from pathlib import Path
from typing import Optional

# Secure base directory for serving images - must be set to a trusted directory
BASE_DIR = Path("./images").resolve()
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB limit

def img(file_param: Optional[str]) -> bytes:
    """\n    Securely read and return image file content.\n    \n    Args:\n        file_param: The requested file path (relative to BASE_DIR)\n        \n    Returns:\n        bytes: The file content\n        \n    Raises:\n        ValueError: If file_param is invalid\n        PermissionError: If path traversal is attempted\n        FileNotFoundError: If file doesn't exist\n        IOError: If file is too large or not a regular file\n    """
    # Input validation: reject None or empty file parameter
    if not file_param or not file_param.strip():
        raise ValueError("File parameter is required")
    
    # Normalize and resolve the requested path against the base directory
    # This prevents path traversal attacks
    requested_path = (BASE_DIR / file_param).resolve()
    
    # Security check: ensure the resolved path is within BASE_DIR
    # This prevents directory traversal attacks like ../../../etc/passwd
    if not str(requested_path).startswith(str(BASE_DIR)):
        raise PermissionError("Access denied: path traversal attempt detected")
    
    # Open the file with secure flags - no symlink following
    # Use os.open with O_RDONLY and O_NOFOLLOW to prevent symlink attacks
    try:
        # Check if path exists without following symlinks
        if not requested_path.exists():
            raise FileNotFoundError("File not found")
        
        # Validate that it's a regular file (not a directory or symlink)\n        if not requested_path.is_file():\n            raise PermissionError("Access denied: not a regular file")\n        \n        # Check for symlinks explicitly\n        if requested_path.is_symlink():\n            raise PermissionError("Access denied: symbolic links not allowed")\n        \n        # Check file size before reading to prevent memory exhaustion\n        file_size = requested_path.stat().st_size\n        if file_size > MAX_FILE_SIZE:\n            raise IOError("File too large")\n        \n        # Open file descriptor with secure flags\n        fd = os.open(\n            requested_path,\n            os.O_RDONLY | os.O_CLOEXEC | getattr(os, 'O_NOFOLLOW', 0)\n        )\n        \n        try:\n            # Validate the opened file descriptor\n            stat_info = os.fstat(fd)\n            if not stat_info.st_mode & 0o100000:  # S_IFREG check\n                raise PermissionError("Access denied: not a regular file")\n            \n            # Read file content using the validated file descriptor\n            with os.fdopen(fd, 'rb') as f:\n                file_content = f.read()\n            \n            return file_content\n        except:\n            os.close(fd)\n            raise\n            \n    except (OSError, IOError) as e:\n        raise IOError(f"Error reading file: {str(e)}")\n\ndef main():\n    """Test cases for the img function"""\n    # Create base directory if it doesn't exist
    BASE_DIR.mkdir(parents=True, exist_ok=True)
    
    print("Test Case 1: Valid image file")
    try:
        # Create a test file
        test_file1 = BASE_DIR / "test1.jpg"
        test_file1.write_bytes(b'\\xFF\\xD8\\xFF\\xE0')
        result1 = img("test1.jpg")
        print(f"Success: Read {len(result1)} bytes")
    except Exception as e:
        print(f"Error: {e}")
    
    print("\\nTest Case 2: Path traversal attempt (should fail)")
    try:
        result2 = img("../../../etc/passwd")
        print("Error: Should have been blocked")
    except PermissionError as e:
        print(f"Success: {e}")
    
    print("\\nTest Case 3: None file parameter (should fail)")
    try:
        result3 = img(None)
        print("Error: Should have been blocked")
    except ValueError as e:
        print(f"Success: {e}")
    
    print("\\nTest Case 4: Non-existent file (should fail)")
    try:
        result4 = img("nonexistent.jpg")
        print("Error: Should have failed")
    except (FileNotFoundError, IOError) as e:
        print(f"Success: {e}")
    
    print("\\nTest Case 5: Empty filename (should fail)")
    try:
        result5 = img("")
        print("Error: Should have been blocked")
    except ValueError as e:
        print(f"Success: {e}")

if __name__ == "__main__":
    main()
