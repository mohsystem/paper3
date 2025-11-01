
import os
import sys
from pathlib import Path

# Maximum file size limit (10MB)
MAX_FILE_SIZE = 10 * 1024 * 1024
BASE_DIR = "/safe/"

def read_safe_file(filename):
    """\n    Securely reads a file from /safe/ directory\n    \n    Args:\n        filename: The name of the file to read (relative to /safe/)\n    \n    Returns:\n        File contents as string, or None on error\n    """
    # Input validation - reject None or empty
    if not filename or not filename.strip():
        print("Invalid filename", file=sys.stderr)
        return None
    
    # Reject obvious traversal attempts
    if ".." in filename or "/" in filename or "\\\\" in filename:\n        print("Invalid filename characters", file=sys.stderr)\n        return None\n    \n    try:\n        # Construct and normalize paths\n        base_path = Path(BASE_DIR).resolve()\n        file_path = (base_path / filename).resolve()\n        \n        # Verify resolved path is within base directory (path traversal check)\n        if not str(file_path).startswith(str(base_path)):\n            print("Path traversal detected", file=sys.stderr)\n            return None\n        \n        # Open file with O_NOFOLLOW to prevent symlink following\n        # Use os.open for low-level control\n        fd = os.open(str(file_path), os.O_RDONLY | os.O_CLOEXEC | (os.O_NOFOLLOW if hasattr(os, 'O_NOFOLLOW') else 0))\n        \n        try:\n            # Validate file descriptor - must be regular file\n            stat_info = os.fstat(fd)\n            \n            # Check if regular file\n            import stat as stat_module\n            if not stat_module.S_ISREG(stat_info.st_mode):\n                print("Not a regular file", file=sys.stderr)\n                os.close(fd)\n                return None\n            \n            # Check size limit\n            if stat_info.st_size > MAX_FILE_SIZE:\n                print("File too large", file=sys.stderr)\n                os.close(fd)\n                return None\n            \n            # Read file using the validated descriptor\n            with os.fdopen(fd, 'rb') as f:\n                content = f.read()\n                return content.decode('utf-8')\n        \n        except Exception:\n            os.close(fd)\n            raise\n            \n    except FileNotFoundError:\n        print("File not found", file=sys.stderr)\n        return None\n    except PermissionError:\n        print("Permission denied", file=sys.stderr)\n        return None\n    except Exception:\n        print("File operation failed", file=sys.stderr)\n        return None\n\ndef main():\n    # Test case 1: Valid file\n    print("Test 1: Valid file")\n    result1 = read_safe_file("test.txt")\n    print(f"Result: {'Success' if result1 is not None else 'Failed'}")\n    \n    # Test case 2: Path traversal attempt\n    print("\
Test 2: Path traversal with ..")\n    result2 = read_safe_file("../etc/passwd")\n    print(f"Result: {'Blocked' if result2 is None else 'Failed'}")\n    \n    # Test case 3: Absolute path attempt\n    print("\
Test 3: Absolute path")\n    result3 = read_safe_file("/etc/passwd")\n    print(f"Result: {'Blocked' if result3 is None else 'Failed'}")\n    \n    # Test case 4: None input\n    print("\
Test 4: None input")\n    result4 = read_safe_file(None)\n    print(f"Result: {'Blocked' if result4 is None else 'Failed'}")\n    \n    # Test case 5: Empty filename\n    print("\
Test 5: Empty filename")\n    result5 = read_safe_file("")\n    print(f"Result: {'Blocked' if result5 is None else 'Failed'}")\n\nif __name__ == "__main__":
    main()
