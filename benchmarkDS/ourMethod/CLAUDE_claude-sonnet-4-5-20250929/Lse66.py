
import os
import secrets
import base64
from pathlib import Path
from typing import BinaryIO

UPLOAD_DIR = "uploads"
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB limit
PDF_MAGIC = b'%PDF'

def upload_pdf(file_stream: BinaryIO, original_filename: str, file_size: int) -> str:
    """\n    Securely upload a PDF file with validation and path traversal prevention.\n    \n    Args:\n        file_stream: Binary file stream to read from\n        original_filename: Original name of the uploaded file\n        file_size: Size of the file in bytes\n    \n    Returns:\n        str: The secure filename used for storage\n    \n    Raises:\n        ValueError: If validation fails\n        IOError: If file operations fail\n    """
    # Input validation: check file size
    if file_size <= 0 or file_size > MAX_FILE_SIZE:
        raise ValueError("Invalid file size")
    
    # Validate filename is not None and doesn't contain path separators\n    if not original_filename or '..' in original_filename or '/' in original_filename or '\\\\' in original_filename:\n        raise ValueError("Invalid filename")\n    \n    # Check file extension\n    if not original_filename.lower().endswith('.pdf'):\n        raise ValueError("Only PDF files are allowed")\n    \n    # Create uploads directory with restricted permissions if it doesn't exist
    upload_path = Path(UPLOAD_DIR).resolve()
    if not upload_path.exists():
        upload_path.mkdir(mode=0o700, parents=True, exist_ok=True)
    
    # Generate secure random filename to prevent collisions and attacks
    random_bytes = secrets.token_bytes(16)
    safe_filename = base64.urlsafe_b64encode(random_bytes).decode('ascii').rstrip('=') + '.pdf'
    
    # Construct target path and validate it's within upload directory\n    target_path = (upload_path / safe_filename).resolve()\n    if not str(target_path).startswith(str(upload_path)):\n        raise SecurityError("Path traversal attempt detected")\n    \n    # Read file content with size limit enforcement\n    file_content = bytearray()\n    chunk_size = 8192\n    total_read = 0\n    \n    while True:\n        chunk = file_stream.read(chunk_size)\n        if not chunk:\n            break\n        total_read += len(chunk)\n        if total_read > MAX_FILE_SIZE:\n            raise IOError("File size exceeds maximum allowed size")\n        file_content.extend(chunk)\n    \n    # Validate PDF magic bytes\n    if len(file_content) < 4:\n        raise ValueError("File too small to be a valid PDF")\n    \n    if not file_content.startswith(PDF_MAGIC):\n        raise ValueError("File is not a valid PDF")\n    \n    # Write to temporary file first, then atomically move to final location\n    temp_fd = None\n    temp_path = None\n    \n    try:\n        # Create temp file with O_EXCL to prevent race conditions\n        temp_fd = os.open(\n            str(upload_path / f"temp_{safe_filename}"),\n            os.O_CREAT | os.O_EXCL | os.O_WRONLY | os.O_CLOEXEC,\n            0o600  # Restrictive permissions: owner read/write only\n        )\n        temp_path = Path(upload_path / f"temp_{safe_filename}")\n        \n        # Write content to temp file\n        bytes_written = 0\n        while bytes_written < len(file_content):\n            n = os.write(temp_fd, file_content[bytes_written:])\n            bytes_written += n\n        \n        # Ensure data is written to disk\n        os.fsync(temp_fd)\n        os.close(temp_fd)\n        temp_fd = None\n        \n        # Atomic move to final location\n        os.replace(str(temp_path), str(target_path))\n        \n        return safe_filename\n        \n    except Exception as e:\n        # Clean up on error\n        if temp_fd is not None:\n            os.close(temp_fd)\n        if temp_path and temp_path.exists():\n            temp_path.unlink()\n        raise\n    finally:\n        if temp_fd is not None:\n            os.close(temp_fd)\n\ndef main():\n    """Test cases for PDF upload functionality"""\n    import io\n    \n    print("PDF Upload Test Cases:")\n    \n    # Test case 1: Valid PDF file\n    try:\n        valid_pdf = bytearray(100)\n        valid_pdf[0:4] = b'%PDF'\n        stream1 = io.BytesIO(bytes(valid_pdf))\n        result1 = upload_pdf(stream1, "document.pdf", len(valid_pdf))\n        print(f"Test 1 PASSED: {result1}")\n    except Exception as e:\n        print(f"Test 1 FAILED: {e}")\n    \n    # Test case 2: File too large\n    try:\n        upload_pdf(io.BytesIO(b''), "large.pdf", 20 * 1024 * 1024)\n        print("Test 2 FAILED: Should reject large file")\n    except ValueError as e:\n        print("Test 2 PASSED: Rejected large file")\n    except Exception as e:\n        print(f"Test 2 FAILED: {e}")\n    \n    # Test case 3: Path traversal attempt\n    try:\n        valid_pdf = bytearray(100)\n        valid_pdf[0:4] = b'%PDF'\n        upload_pdf(io.BytesIO(bytes(valid_pdf)), "../../../etc/passwd.pdf", len(valid_pdf))\n        print("Test 3 FAILED: Should reject path traversal")\n    except ValueError as e:\n        print("Test 3 PASSED: Rejected path traversal")\n    except Exception as e:\n        print(f"Test 3 FAILED: {e}")\n    \n    # Test case 4: Invalid file extension\n    try:\n        valid_pdf = bytearray(100)\n        valid_pdf[0:4] = b'%PDF'
        upload_pdf(io.BytesIO(bytes(valid_pdf)), "document.exe", len(valid_pdf))
        print("Test 4 FAILED: Should reject non-PDF extension")
    except ValueError as e:
        print("Test 4 PASSED: Rejected non-PDF extension")
    except Exception as e:
        print(f"Test 4 FAILED: {e}")
    
    # Test case 5: Invalid PDF magic bytes
    try:
        invalid_pdf = bytearray(100)
        upload_pdf(io.BytesIO(bytes(invalid_pdf)), "fake.pdf", len(invalid_pdf))
        print("Test 5 FAILED: Should reject invalid PDF")
    except ValueError as e:
        print("Test 5 PASSED: Rejected invalid PDF magic bytes")
    except Exception as e:
        print(f"Test 5 FAILED: {e}")

if __name__ == "__main__":
    main()
