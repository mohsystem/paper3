
import os
import sys
import secrets
from pathlib import Path
from typing import Optional

# Security: Maximum file size to prevent DoS and memory exhaustion (CWE-400)
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10 MB
# Security: Maximum filename length to prevent buffer issues (CWE-120)
MAX_FILENAME_LEN = 255
# Security: Base directory for uploads - prevents path traversal (CWE-22)
BASE_UPLOAD_DIR = "./uploads"


def validate_filename(filename: str) -> bool:
    """\n    Security: Validates filename to prevent path traversal attacks (CWE-22, CWE-73)\n    Rejects: "..", "/", "\\\\", null bytes, and control characters\n    Only allows: safe alphanumeric and basic punctuation\n    """
    # Security: Type and null check (CWE-20)
    if not filename or not isinstance(filename, str):
        print("Error: Invalid filename type", file=sys.stderr)
        return False
    
    # Security: Length validation (CWE-120, CWE-131)
    if len(filename) == 0 or len(filename) > MAX_FILENAME_LEN:
        print("Error: Invalid filename length", file=sys.stderr)
        return False
    
    # Security: Reject hidden files and files starting with path separators (CWE-22)
    if filename[0] in ('.', '/', '\\\\'):\n        print("Error: Filename cannot start with '.', '/', or '\\\\'", file=sys.stderr)\n        return False\n    \n    # Security: Check for null bytes (CWE-158)\n    if '\\0' in filename:\n        print("Error: Null byte in filename", file=sys.stderr)\n        return False\n    \n    # Security: Check for path traversal sequences (CWE-22)\n    if '..' in filename or '/' in filename or '\\\\' in filename:\n        print("Error: Path traversal characters not allowed", file=sys.stderr)\n        return False\n    \n    # Security: Reject control characters (CWE-20)\n    for char in filename:\n        if ord(char) < 32 or ord(char) == 127:\n            print("Error: Control characters not allowed", file=sys.stderr)\n            return False\n    \n    # Security: Additional dangerous filename checks\n    if filename in ('.', '..'):\n        print("Error: Invalid filename", file=sys.stderr)\n        return False\n    \n    return True\n\n\ndef construct_safe_path(filename: str) -> Optional[Path]:\n    """\n    Security: Constructs and validates safe file path within base directory (CWE-22, CWE-73)\n    Returns None if path would escape base directory\n    """\n    # Security: Validate filename first (CWE-20)\n    if not validate_filename(filename):\n        return None\n    \n    try:\n        # Security: Construct path and resolve it to absolute path\n        base_path = Path(BASE_UPLOAD_DIR).resolve()\n        target_path = (base_path / filename).resolve()\n        \n        # Security: Ensure target path is within base directory (CWE-22)\n        # This prevents path traversal even with symlinks\n        if not str(target_path).startswith(str(base_path) + os.sep):\n            print("Error: Path traversal detected", file=sys.stderr)\n            return None\n        \n        # Security: Additional check using relative_to\n        try:\n            target_path.relative_to(base_path)\n        except ValueError:\n            print("Error: Path outside base directory", file=sys.stderr)\n            return None\n        \n        return target_path\n        \n    except (OSError, ValueError) as e:\n        print(f"Error: Path construction failed: {e}", file=sys.stderr)\n        return None\n\n\ndef ensure_upload_directory() -> bool:\n    """\n    Security: Creates upload directory with restrictive permissions (CWE-732)\n    Directory mode 0o700 allows only owner access\n    """\n    try:\n        upload_dir = Path(BASE_UPLOAD_DIR)\n        \n        if upload_dir.exists():\n            # Security: Verify it's actually a directory (CWE-22)
            if not upload_dir.is_dir():
                print("Error: Upload path exists but is not a directory", file=sys.stderr)
                return False
            return True
        
        # Security: Create directory with restrictive permissions (CWE-732)
        upload_dir.mkdir(mode=0o700, parents=False, exist_ok=False)
        return True
        
    except OSError as e:
        print(f"Error: Failed to create upload directory: {e}", file=sys.stderr)
        return False


def upload_file_safe(filename: str, data: bytes) -> bool:
    """\n    Security: Race-condition safe file upload using open-then-validate pattern (CWE-367, CWE-363)\n    Uses O_CREAT|O_EXCL flags to prevent overwriting and TOCTOU issues (CWE-59)\n    """
    # Security: Input validation (CWE-20)
    if not filename or not isinstance(filename, str):
        print("Error: Invalid filename", file=sys.stderr)
        return False
    
    if not isinstance(data, bytes):
        print("Error: Data must be bytes", file=sys.stderr)
        return False
    
    # Security: Validate file size to prevent DoS (CWE-400)
    if len(data) == 0 or len(data) > MAX_FILE_SIZE:
        print("Error: Invalid file size", file=sys.stderr)
        return False
    
    # Security: Ensure upload directory exists with proper permissions
    if not ensure_upload_directory():
        return False
    
    # Security: Construct and validate safe path (CWE-22)
    target_path = construct_safe_path(filename)
    if target_path is None:
        return False
    
    fd = None
    try:
        # Security: Open file with exclusive creation (CWE-367, CWE-59, CWE-732)
        # os.O_CREAT | os.O_EXCL: Fails if file exists (prevents race conditions)
        # os.O_WRONLY: Write-only access
        # os.O_CLOEXEC: Close on exec for security
        # Mode 0o600: Owner read/write only (restrictive permissions)
        fd = os.open(
            target_path,
            os.O_CREAT | os.O_EXCL | os.O_WRONLY | os.O_CLOEXEC,
            mode=0o600
        )
        
        # Security: Validate the opened file descriptor (CWE-367)
        # This prevents TOCTOU by checking the already-opened file
        stat_info = os.fstat(fd)
        
        # Security: Ensure it's a regular file (CWE-59)\n        import stat\n        if not stat.S_ISREG(stat_info.st_mode):\n            print("Error: Not a regular file", file=sys.stderr)\n            os.close(fd)\n            os.unlink(target_path)\n            return False\n        \n        # Security: Write data and check result (CWE-252)\n        bytes_written = os.write(fd, data)\n        \n        if bytes_written != len(data):\n            print("Error: Incomplete write", file=sys.stderr)\n            os.close(fd)\n            os.unlink(target_path)\n            return False\n        \n        # Security: Ensure data is written to disk (CWE-662)\n        os.fsync(fd)\n        \n        # Security: Close file descriptor (CWE-404)\n        os.close(fd)\n        fd = None\n        \n        print(f"Success: File '{filename}' uploaded successfully ({len(data)} bytes)")\n        return True\n        \n    except FileExistsError:\n        print("Error: File already exists", file=sys.stderr)\n        if fd is not None:\n            os.close(fd)\n        return False\n        \n    except OSError as e:\n        print(f"Error: File operation failed: {e}", file=sys.stderr)\n        if fd is not None:\n            os.close(fd)\n        # Security: Clean up on error\n        try:\n            if target_path and target_path.exists():\n                os.unlink(target_path)\n        except OSError:\n            pass\n        return False\n        \n    except Exception as e:\n        print(f"Error: Unexpected error: {e}", file=sys.stderr)\n        if fd is not None:\n            os.close(fd)\n        # Security: Clean up on error\n        try:\n            if target_path and target_path.exists():\n                os.unlink(target_path)\n        except OSError:\n            pass\n        return False\n\n\ndef main():\n    """Test function with 5 test cases"""\n    print("=== File Upload Server Test Cases ===\
")\n    \n    # Test Case 1: Normal file upload\n    print("Test 1: Normal file upload")\n    test_data1 = b"This is test file content for test case 1."\n    if upload_file_safe("test1.txt", test_data1):\n        print("Test 1: PASSED\
")\n    else:\n        print("Test 1: FAILED\
")\n    \n    # Test Case 2: Different content file\n    print("Test 2: Different content file")\n    test_data2 = b"Test case 2: Different content with more data to test handling."\n    if upload_file_safe("test2.txt", test_data2):\n        print("Test 2: PASSED\
")\n    else:\n        print("Test 2: FAILED\
")\n    \n    # Test Case 3: Small file\n    print("Test 3: Small file upload")\n    test_data3 = b"Small"\n    if upload_file_safe("small_file.txt", test_data3):\n        print("Test 3: PASSED\
")\n    else:\n        print("Test 3: FAILED\
")\n    \n    # Test Case 4: Larger file\n    print("Test 4: Larger file upload")\n    # Security: Use secrets module for generating random test data\n    large_data = bytes([(65 + (i % 26)) for i in range(1024)])\n    if upload_file_safe("large_test.txt", large_data):\n        print("Test 4: PASSED\
")\n    else:\n        print("Test 4: FAILED\
")\n    \n    # Test Case 5: Security test - path traversal attempt (should fail)\n    print("Test 5: Security test - path traversal attempt (should fail)")\n    test_data5 = b"Malicious content"\n    if not upload_file_safe("../etc/passwd", test_data5):\n        print("Test 5: PASSED (correctly rejected malicious filename)\
")\n    else:\n        print("Test 5: FAILED (security vulnerability detected!)\
")\n    \n    print("=== All tests completed ===")\n\n\nif __name__ == "__main__":
    main()
