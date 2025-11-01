
import os
import re
import secrets
import time
from pathlib import Path

MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'jpg', 'jpeg', 'png', 'gif'}
UPLOAD_DIR = 'uploads'

def upload_file(file_data, original_filename):
    try:
        # Create uploads directory if it doesn't exist\n        upload_path = Path(UPLOAD_DIR)\n        upload_path.mkdir(exist_ok=True)\n        \n        # Validate file size\n        if len(file_data) > MAX_FILE_SIZE:\n            return "Error: File size exceeds limit"\n        \n        # Sanitize and validate filename\n        sanitized_name = sanitize_filename(original_filename)\n        if not sanitized_name:\n            return "Error: Invalid filename"\n        \n        # Validate file extension\n        extension = get_file_extension(sanitized_name)\n        if extension.lower() not in ALLOWED_EXTENSIONS:\n            return "Error: File type not allowed"\n        \n        # Generate unique filename\n        unique_filename = generate_unique_filename(sanitized_name)\n        file_path = upload_path / unique_filename\n        \n        # Prevent path traversal\n        try:\n            file_path = file_path.resolve()\n            upload_path = upload_path.resolve()\n            if not str(file_path).startswith(str(upload_path)):\n                return "Error: Invalid file path"\n        except Exception:\n            return "Error: Invalid file path"\n        \n        # Save file\n        with open(file_path, 'wb') as f:\n            f.write(file_data)\n        \n        return f"Success: File uploaded as {unique_filename}"\n        \n    except Exception as e:\n        return f"Error: {str(e)}"\n\ndef sanitize_filename(filename):\n    if not filename:\n        return None\n    return re.sub(r'[^a-zA-Z0-9._-]', '_', filename)\n\ndef get_file_extension(filename):\n    if '.' in filename:\n        return filename.rsplit('.', 1)[1]\n    return ''\n\ndef generate_unique_filename(original_name):\n    extension = get_file_extension(original_name)\n    name_without_ext = original_name.rsplit('.', 1)[0]\n    timestamp = int(time.time() * 1000)\n    random_num = secrets.randbelow(10000)\n    return f"{name_without_ext}_{timestamp}_{random_num}.{extension}"\n\nif __name__ == "__main__":\n    print("=== File Upload Test Cases ===\\n")\n    \n    # Test case 1: Valid text file\n    test_file1 = b"Hello, World!"\n    print("Test 1 - Valid text file:", upload_file(test_file1, "test.txt"))\n    \n    # Test case 2: Valid image file\n    test_file2 = b'\\x00' * 1024
    print("Test 2 - Valid image file:", upload_file(test_file2, "image.jpg"))
    
    # Test case 3: Invalid extension
    test_file3 = b"script content"
    print("Test 3 - Invalid extension:", upload_file(test_file3, "malicious.exe"))
    
    # Test case 4: Path traversal attempt
    test_file4 = b"data"
    print("Test 4 - Path traversal attempt:", upload_file(test_file4, "../../../etc/passwd.txt"))
    
    # Test case 5: Empty filename
    test_file5 = b"content"
    print("Test 5 - Empty filename:", upload_file(test_file5, ""))
