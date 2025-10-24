
import os
import re
import time
import hashlib
from pathlib import Path
from typing import Tuple, Optional

MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'jpg', 'jpeg', 'png', 'doc', 'docx'}
UPLOAD_DIRECTORY = 'uploads'
SAFE_FILENAME_PATTERN = re.compile(r'^[a-zA-Z0-9_\\-\\.]+$')


def upload_file(filename: str, file_content: bytes) -> Tuple[bool, str, Optional[str]]:
    """\n    Upload a file securely with validation\n    \n    Args:\n        filename: Name of the file to upload\n        file_content: Content of the file as bytes\n        \n    Returns:\n        Tuple of (success: bool, message: str, saved_path: str or None)\n    """
    try:
        # Validate filename
        if not filename or not filename.strip():
            return False, "Invalid filename", None
        
        # Sanitize filename
        sanitized_filename = sanitize_filename(filename)
        if sanitized_filename is None:
            return False, "Filename contains invalid characters", None
        
        # Check file extension
        extension = get_file_extension(sanitized_filename)
        if extension.lower() not in ALLOWED_EXTENSIONS:
            return False, "File type not allowed", None
        
        # Validate file size
        if file_content is None or len(file_content) == 0:
            return False, "Empty file", None
        if len(file_content) > MAX_FILE_SIZE:
            return False, "File size exceeds maximum limit", None
        
        # Create upload directory if it doesn't exist\n        upload_path = Path(UPLOAD_DIRECTORY)\n        upload_path.mkdir(parents=True, exist_ok=True)\n        \n        # Generate unique filename\n        unique_filename = generate_unique_filename(sanitized_filename)\n        file_path = upload_path / unique_filename\n        \n        # Prevent path traversal\n        if not file_path.resolve().is_relative_to(upload_path.resolve()):\n            return False, "Invalid file path", None\n        \n        # Write file\n        with open(file_path, 'wb') as f:\n            f.write(file_content)\n        \n        return True, "File uploaded successfully", str(file_path)\n        \n    except Exception as e:\n        return False, f"Error uploading file: {str(e)}", None\n\n\ndef sanitize_filename(filename: str) -> Optional[str]:\n    """Remove path components and validate filename"""\n    # Get base filename\n    base_name = os.path.basename(filename)\n    \n    # Check if filename matches safe pattern\n    if not SAFE_FILENAME_PATTERN.match(base_name):\n        return None\n    \n    return base_name\n\n\ndef get_file_extension(filename: str) -> str:\n    """Extract file extension"""\n    if '.' in filename:\n        return filename.rsplit('.', 1)[1]\n    return ""\n\n\ndef generate_unique_filename(original_filename: str) -> str:\n    """Generate unique filename with timestamp"""\n    timestamp = str(int(time.time() * 1000))\n    name_without_ext = original_filename.rsplit('.', 1)[0]
    extension = get_file_extension(original_filename)
    return f"{name_without_ext}_{timestamp}.{extension}"


def main():
    print("=== File Upload Program - Test Cases ===\\n")
    
    # Test Case 1: Valid text file
    print("Test Case 1: Valid text file")
    content1 = b"Hello, this is a test file."
    success1, message1, path1 = upload_file("test.txt", content1)
    print(f"Success: {success1}")
    print(f"Message: {message1}")
    print(f"Path: {path1}\\n")
    
    # Test Case 2: Valid PDF file
    print("Test Case 2: Valid PDF file")
    content2 = bytes([0x50] * 1024)
    success2, message2, path2 = upload_file("document.pdf", content2)
    print(f"Success: {success2}")
    print(f"Message: {message2}")
    print(f"Path: {path2}\\n")
    
    # Test Case 3: Invalid file extension
    print("Test Case 3: Invalid file extension")
    content3 = b"Executable content"
    success3, message3, path3 = upload_file("malicious.exe", content3)
    print(f"Success: {success3}")
    print(f"Message: {message3}\\n")
    
    # Test Case 4: File too large
    print("Test Case 4: File too large")
    content4 = bytes(MAX_FILE_SIZE + 1)
    success4, message4, path4 = upload_file("large_file.txt", content4)
    print(f"Success: {success4}")
    print(f"Message: {message4}\\n")
    
    # Test Case 5: Path traversal attempt
    print("Test Case 5: Path traversal attempt")
    content5 = b"Malicious content"
    success5, message5, path5 = upload_file("../../etc/passwd.txt", content5)
    print(f"Success: {success5}")
    print(f"Message: {message5}\\n")


if __name__ == "__main__":
    main()
