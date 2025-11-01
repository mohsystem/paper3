
import os
import secrets
import hashlib
from pathlib import Path
from typing import BinaryIO, Optional

# Maximum file size: 10MB
MAX_FILE_SIZE = 10 * 1024 * 1024

# Allowed file extensions (whitelist)
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'jpg', 'jpeg', 'png', 'doc', 'docx'}

# Upload directory name
UPLOAD_DIR = 'uploads'


def upload_file(filename: str, file_content: BinaryIO) -> str:
    """\n    Securely upload a file to the uploads directory.\n    \n    Args:\n        filename: Original filename provided by user\n        file_content: File content as binary stream\n        \n    Returns:\n        The saved filename\n        \n    Raises:\n        ValueError: If validation fails\n        IOError: If file operations fail\n    """
    # Input validation: reject null or empty filename
    if not filename or not filename.strip():
        raise ValueError("Invalid filename")
    
    # Sanitize filename to prevent path traversal
    sanitized_filename = sanitize_filename(filename)
    if not sanitized_filename:
        raise ValueError("Invalid filename after sanitization")
    
    # Validate file extension against whitelist
    extension = get_file_extension(sanitized_filename)
    if extension.lower() not in ALLOWED_EXTENSIONS:
        raise ValueError(f"File type not allowed: {extension}")
    
    # Get absolute, normalized base directory path
    base_dir = Path.cwd() / UPLOAD_DIR
    base_dir = base_dir.resolve()
    
    # Create uploads directory with restrictive permissions if it doesn't exist\n    if not base_dir.exists():\n        base_dir.mkdir(mode=0o700, parents=True)\n    \n    # Generate unique filename to prevent collisions\n    unique_filename = generate_unique_filename(sanitized_filename)\n    target_path = (base_dir / unique_filename).resolve()\n    \n    # Verify resolved path is within upload directory (prevent traversal)\n    if not str(target_path).startswith(str(base_dir)):\n        raise ValueError("Path traversal attempt detected")\n    \n    # Open file descriptor with O_CREAT|O_EXCL to prevent TOCTOU and race conditions\n    # O_EXCL ensures file doesn't exist, O_CREAT creates it atomically
    try:
        fd = os.open(
            str(target_path),
            os.O_WRONLY | os.O_CREAT | os.O_EXCL,
            mode=0o600  # Restrictive permissions: owner read/write only
        )
    except FileExistsError:
        raise IOError("File already exists")
    
    try:
        bytes_written = 0
        buffer_size = 8192
        
        # Write file content with size limit
        with os.fdopen(fd, 'wb') as f:
            while True:
                chunk = file_content.read(buffer_size)
                if not chunk:
                    break
                
                bytes_written += len(chunk)
                if bytes_written > MAX_FILE_SIZE:
                    raise ValueError("File size exceeds maximum allowed size")
                
                f.write(chunk)
            
            # Ensure data is flushed to disk
            f.flush()
            os.fsync(f.fileno())
        
        return unique_filename
        
    except Exception as e:
        # Clean up on error: remove partially written file
        try:
            os.unlink(str(target_path))
        except OSError:
            pass
        raise e


def sanitize_filename(filename: str) -> str:
    """\n    Sanitize filename to prevent path traversal and remove dangerous characters.\n    """
    # Remove null bytes, path separators, and control characters
    sanitized = filename.replace('\\x00', '')
    sanitized = sanitized.replace('/', '').replace('\\\\', '')\n    \n    # Remove leading/trailing dots and spaces (Windows restrictions)\n    sanitized = sanitized.strip('. ')\n    \n    # Limit length to prevent filesystem issues\n    if len(sanitized) > 255:\n        # Preserve extension\n        name_part = sanitized[:250]\n        ext_part = sanitized[-5:] if '.' in sanitized[-5:] else ''\n        sanitized = name_part + ext_part\n    \n    return sanitized\n\n\ndef get_file_extension(filename: str) -> str:\n    """Extract file extension from filename."""\n    parts = filename.rsplit('.', 1)\n    if len(parts) == 2 and parts[1]:\n        return parts[1]\n    return ''\n\n\ndef generate_unique_filename(original_filename: str) -> str:\n    """Generate a unique filename using secure random bytes."""\n    # Generate 16 random bytes for uniqueness\n    random_hex = secrets.token_hex(16)\n    \n    # Extract base name and extension\n    extension = get_file_extension(original_filename)\n    base_name = original_filename.rsplit('.', 1)[0]\n    \n    # Limit base name length\n    if len(base_name) > 50:\n        base_name = base_name[:50]\n    \n    return f"{base_name}_{random_hex}.{extension}"\n\n\ndef main():\n    """Test cases for upload_file function."""\n    \n    # Test case 1: Valid text file\n    try:\n        from io import BytesIO\n        content1 = BytesIO(b"Test file content")\n        result1 = upload_file("test.txt", content1)\n        print(f"Test 1 passed: {result1}")\n    except Exception as e:\n        print(f"Test 1 failed: {e}")\n    \n    # Test case 2: Path traversal attempt (should fail)\n    try:\n        from io import BytesIO\n        content2 = BytesIO(b"Malicious content")\n        result2 = upload_file("../../../etc/passwd.txt", content2)\n        print("Test 2 failed: Should have rejected path traversal")\n    except Exception as e:\n        print(f"Test 2 passed: Rejected path traversal - {e}")\n    \n    # Test case 3: Invalid file extension (should fail)\n    try:\n        from io import BytesIO\n        content3 = BytesIO(b"<?php system($_GET['cmd']); ?>")\n        result3 = upload_file("shell.php", content3)\n        print("Test 3 failed: Should have rejected .php extension")\n    except Exception as e:\n        print(f"Test 3 passed: Rejected invalid extension - {e}")\n    \n    # Test case 4: Valid PDF file\n    try:\n        from io import BytesIO\n        content4 = BytesIO(b"%PDF-1.4 test")\n        result4 = upload_file("document.pdf", content4)\n        print(f"Test 4 passed: {result4}")\n    except Exception as e:\n        print(f"Test 4 failed: {e}")\n    \n    # Test case 5: Empty filename (should fail)\n    try:\n        from io import BytesIO\n        content5 = BytesIO(b"Content")\n        result5 = upload_file("", content5)\n        print("Test 5 failed: Should have rejected empty filename")\n    except Exception as e:\n        print(f"Test 5 passed: Rejected empty filename - {e}")\n\n\nif __name__ == "__main__":
    main()
