
import os
import re
import tempfile
import shutil
from pathlib import Path
from typing import BinaryIO, Optional

# Maximum file size: 10MB
MAX_FILE_SIZE = 10 * 1024 * 1024
# Maximum filename length
MAX_FILENAME_LEN = 255
# Buffer size for reading chunks
BUFFER_SIZE = 8192
# Pattern for valid filenames: alphanumeric, dash, underscore, dot
VALID_FILENAME_PATTERN = re.compile(r'^[a-zA-Z0-9][a-zA-Z0-9._-]*$')


def validate_filename(filename: str) -> bool:
    """\n    Validates filename to prevent path traversal and ensure safe characters.\n    Rejects null, empty, too long, path separators, parent directory references,\n    and filenames starting with a dot.\n    """
    if not filename or not isinstance(filename, str):
        return False
    
    if len(filename) > MAX_FILENAME_LEN:
        return False
    
    # Check for path traversal attempts
    if '..' in filename or '/' in filename or '\\\\' in filename:\n        return False\n    \n    # Check against allowed character pattern\n    if not VALID_FILENAME_PATTERN.match(filename):\n        return False\n    \n    # Must not start with a dot\n    if filename.startswith('.'):\n        return False\n    \n    return True\n\n\ndef validate_upload_directory(base_dir_str: str) -> Path:\n    """\n    Validates that the upload directory exists, is a directory, and returns\n    its normalized canonical path.\n    """\n    if not base_dir_str:\n        raise ValueError("Base directory cannot be None or empty")\n    \n    base_dir = Path(base_dir_str)\n    \n    # Check directory exists\n    if not base_dir.exists():\n        raise IOError(f"Upload directory does not exist: {base_dir_str}")\n    \n    # Check it's a directory
    if not base_dir.is_dir():
        raise IOError(f"Upload path is not a directory: {base_dir_str}")
    
    # Resolve to canonical path (follows symlinks)
    try:
        base_dir = base_dir.resolve(strict=True)
    except (OSError, RuntimeError) as e:
        raise IOError(f"Cannot resolve directory path: {e}")
    
    return base_dir


def construct_safe_path(base_dir: Path, filename: str) -> Path:
    """\n    Constructs and validates the full file path, ensuring it resolves\n    within the base directory after normalization.\n    """
    # Construct the full path
    full_path = base_dir / filename
    
    # Resolve to canonical path
    try:
        # Use resolve() to normalize the path
        full_path = full_path.resolve()
    except (OSError, RuntimeError):
        # If file doesn't exist yet, that's OK, but verify parent
        full_path = full_path.absolute()
    
    # Verify the path is within the base directory
    try:
        full_path.relative_to(base_dir)
    except ValueError:
        raise SecurityError("Path traversal attempt detected")
    
    # If file exists and is a symlink, reject it
    if full_path.exists() and full_path.is_symlink():
        raise SecurityError("Symlinks are not allowed")
    
    return full_path


class SecurityError(Exception):
    """Custom exception for security-related errors."""
    pass


def upload_file(base_dir_str: str, filename: str, source: BinaryIO) -> bool:
    """\n    Uploads a file with comprehensive security checks.\n    Uses atomic write operation: write to temp file, flush, fsync, then atomic move.\n    \n    Args:\n        base_dir_str: Base directory for uploads\n        filename: Name of the file to upload\n        source: Binary input stream containing file data\n        \n    Returns:\n        True if upload successful, False otherwise\n    """
    temp_path: Optional[Path] = None
    temp_file = None
    
    try:
        # Validate inputs
        if source is None or base_dir_str is None or filename is None:
            print("Error: Invalid input parameters", flush=True)
            return False
        
        # Validate filename
        if not validate_filename(filename):
            print(f"Error: Invalid filename: {filename}", flush=True)
            return False
        
        # Validate and normalize base directory
        base_dir = validate_upload_directory(base_dir_str)
        
        # Construct safe target path
        target_path = construct_safe_path(base_dir, filename)
        
        # Create temporary file in same directory for atomic move
        # Using NamedTemporaryFile with delete=False for manual control
        temp_file = tempfile.NamedTemporaryFile(
            mode='wb',
            dir=base_dir,
            prefix='.tmp_',
            suffix='.upload',
            delete=False
        )
        temp_path = Path(temp_file.name)
        
        # Set secure permissions on temp file (owner read/write only)
        os.chmod(temp_path, 0o600)
        
        # Copy data with size validation
        total_bytes = 0
        
        while True:
            chunk = source.read(BUFFER_SIZE)
            if not chunk:
                break
            
            # Check size limit before writing
            chunk_size = len(chunk)
            if total_bytes > MAX_FILE_SIZE - chunk_size:
                print("Error: File exceeds maximum size limit", flush=True)
                return False
            
            temp_file.write(chunk)
            total_bytes += chunk_size
        
        # Flush to OS buffer
        temp_file.flush()
        
        # Sync to disk (fsync)
        os.fsync(temp_file.fileno())
        
        # Close temp file
        temp_file.close()
        temp_file = None
        
        # Atomically move temp file to target (replaces if exists)
        # On POSIX systems, rename is atomic
        shutil.move(str(temp_path), str(target_path))
        temp_path = None  # Successfully moved, don't delete\n        \n        # Set secure permissions on final file\n        os.chmod(target_path, 0o600)\n        \n        print(f"File uploaded successfully: {filename} ({total_bytes} bytes)", flush=True)\n        return True\n        \n    except SecurityError as e:\n        print(f"Security error: {e}", flush=True)\n        return False\n    except (IOError, OSError, ValueError) as e:\n        print(f"Error: {e}", flush=True)\n        return False\n    except Exception as e:\n        print(f"Unexpected error: {e}", flush=True)\n        return False\n    finally:\n        # Clean up resources\n        if temp_file is not None:\n            try:\n                temp_file.close()\n            except Exception:\n                pass  # Ignore close errors in finally\n        \n        # Delete temp file if it still exists (upload failed)\n        if temp_path is not None and temp_path.exists():\n            try:\n                temp_path.unlink()\n            except Exception:\n                pass  # Ignore deletion errors in finally\n\n\ndef main() -> None:\n    """Main function with test cases demonstrating secure file upload."""\n    upload_dir = "./uploads"\n    \n    # Create upload directory if it doesn't exist
    try:
        Path(upload_dir).mkdir(mode=0o700, exist_ok=True)
    except OSError as e:
        print(f"Failed to create upload directory: {e}")
        return
    
    print("=== File Upload Security Test Cases ===\\n")
    
    # Test 1: Valid file upload
    print("Test 1: Valid file upload")
    try:
        test_data = b"This is a test file for upload."
        from io import BytesIO
        test1 = BytesIO(test_data)
        success = upload_file(upload_dir, "valid_file.txt", test1)
        print(f"Result: {'PASS' if success else 'FAIL'}\\n")
    except Exception as e:
        print(f"Result: FAIL - {e}\\n")
    
    # Test 2: Path traversal attempt (should fail)
    print("Test 2: Path traversal attempt (../etc/passwd)")
    try:
        test_data = b"Malicious content"
        test2 = BytesIO(test_data)
        success = upload_file(upload_dir, "../etc/passwd", test2)
        print(f"Result: {'FAIL - SECURITY ISSUE' if success else 'PASS'} (Expected: FAIL)\\n")
    except Exception as e:
        print(f"Result: PASS (Exception caught)\\n")
    
    # Test 3: Invalid characters in filename (should fail)
    print("Test 3: Invalid characters in filename")
    try:
        test_data = b"Test content"
        test3 = BytesIO(test_data)
        success = upload_file(upload_dir, "file<>name.txt", test3)
        print(f"Result: {'FAIL - SECURITY ISSUE' if success else 'PASS'} (Expected: FAIL)\\n")
    except Exception as e:
        print(f"Result: PASS (Exception caught)\\n")
    
    # Test 4: Hidden file attempt (should fail)
    print("Test 4: Hidden file attempt (.hidden)")
    try:
        test_data = b"Hidden content"
        test4 = BytesIO(test_data)
        success = upload_file(upload_dir, ".hidden", test4)
        print(f"Result: {'FAIL - SECURITY ISSUE' if success else 'PASS'} (Expected: FAIL)\\n")
    except Exception as e:
        print(f"Result: PASS (Exception caught)\\n")
    
    # Test 5: Long but valid filename
    print("Test 5: Long but valid filename")
    try:
        test_data = b"Content for long filename"
        test5 = BytesIO(test_data)
        long_name = 'a' * 200 + '.txt'
        success = upload_file(upload_dir, long_name, test5)
        print(f"Result: {'PASS' if success else 'FAIL'}\\n")
    except Exception as e:
        print(f"Result: FAIL - {e}\\n")
    
    print("=== Tests Complete ===")


if __name__ == "__main__":
    main()
