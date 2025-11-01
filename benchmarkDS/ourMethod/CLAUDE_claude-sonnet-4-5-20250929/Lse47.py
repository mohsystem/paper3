
import os
from pathlib import Path

BASE_DIR = "images"
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB limit

def validate_and_resolve_path(filename):
    """\n    Validates and safely resolves a file path within the base directory.\n    Returns None if the path is invalid or outside the base directory.\n    """
    if not filename or not isinstance(filename, str):
        return None
    
    # Reject path traversal patterns
    if ".." in filename or "/" in filename or "\\\\" in filename:\n        return None\n    \n    try:\n        # Get absolute path of base directory\n        base_path = Path(BASE_DIR).resolve()\n        \n        # Resolve the requested path\n        requested_path = (base_path / filename).resolve()\n        \n        # Ensure the resolved path is within the base directory\n        if not str(requested_path).startswith(str(base_path)):\n            return None\n        \n        return requested_path\n    except (OSError, ValueError):\n        return None\n\ndef serve_image(filename):\n    """
    Handles image file requests with secure path validation.
    This would typically be a route handler in a web framework.
    """\n    file_path = validate_and_resolve_path(filename)\n    \n    if file_path is None:\n        return None\n    \n    try:\n        # Open the file with O_RDONLY and O_NOFOLLOW to prevent symlink following\n        fd = os.open(file_path, os.O_RDONLY | os.O_CLOEXEC)\n        \n        # Validate the opened file descriptor\n        stat_info = os.fstat(fd)\n        \n        # Check if it's a regular file (not directory or symlink)\n        import stat\n        if not stat.S_ISREG(stat_info.st_mode):\n            os.close(fd)\n            return None\n        \n        # Check file size\n        if stat_info.st_size > MAX_FILE_SIZE:\n            os.close(fd)\n            return None\n        \n        # Read file using the validated file descriptor\n        with os.fdopen(fd, 'rb') as f:\n            content = f.read()\n        \n        return content\n    except (OSError, IOError):\n        return None\n\ndef main():\n    """Test cases for the serve_image function"""\n    # Create images directory if it doesn't exist\n    os.makedirs(BASE_DIR, exist_ok=True)\n    \n    # Test case setup - create test files\n    try:\n        with open(os.path.join(BASE_DIR, "test1.jpg"), "wb") as f:\n            f.write(b"test image 1")\n        with open(os.path.join(BASE_DIR, "test2.png"), "wb") as f:\n            f.write(b"test image 2")\n        with open(os.path.join(BASE_DIR, "valid.gif"), "wb") as f:\n            f.write(b"test image 3")\n    except IOError:\n        print("Failed to create test files")\n    \n    # Test cases\n    print("Test 1 - Valid file:")\n    result1 = serve_image("test1.jpg")\n    print("SUCCESS - File served" if result1 is not None else "FAILED")\n    \n    print("\
Test 2 - Another valid file:")\n    result2 = serve_image("test2.png")\n    print("SUCCESS - File served" if result2 is not None else "FAILED")\n    \n    print("\
Test 3 - Non-existent file:")\n    result3 = serve_image("nonexistent.jpg")\n    print("SUCCESS - Rejected" if result3 is None else "FAILED")\n    \n    print("\
Test 4 - Path traversal attempt:")\n    result4 = serve_image("../etc/passwd")\n    print("SUCCESS - Rejected" if result4 is None else "FAILED")\n    \n    print("\
Test 5 - Path with slash:")\n    result5 = serve_image("subdir/file.jpg")\n    print("SUCCESS - Rejected" if result5 is None else "FAILED")\n\nif __name__ == "__main__":
    main()
