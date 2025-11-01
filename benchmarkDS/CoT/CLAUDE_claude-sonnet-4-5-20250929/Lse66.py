
import os
import re
import uuid
from pathlib import Path

UPLOAD_DIR = "uploads"
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB
SAFE_FILENAME_PATTERN = re.compile(r'^[a-zA-Z0-9_\\-\\.]+$')

def upload_pdf_file(file_content, original_filename):
    try:
        # Validate file size
        if not file_content or len(file_content) == 0:
            return "Error: File is empty"
        if len(file_content) > MAX_FILE_SIZE:
            return "Error: File size exceeds limit"
        
        # Validate filename
        if not original_filename:
            return "Error: Invalid filename"
        
        # Validate PDF extension
        if not original_filename.lower().endswith('.pdf'):
            return "Error: Only PDF files are allowed"
        
        # Validate PDF magic bytes
        if not is_pdf_file(file_content):
            return "Error: File is not a valid PDF"
        
        # Create uploads directory if not exists
        upload_path = Path(UPLOAD_DIR).resolve()
        upload_path.mkdir(parents=True, exist_ok=True)
        
        # Generate safe filename
        safe_filename = sanitize_filename(original_filename)
        unique_filename = f"{uuid.uuid4()}_{safe_filename}"
        
        # Resolve target path and validate it's within upload directory\n        target_path = (upload_path / unique_filename).resolve()\n        if not str(target_path).startswith(str(upload_path)):\n            return "Error: Invalid file path"\n        \n        # Write file securely\n        with open(target_path, 'wb') as f:\n            f.write(file_content)\n        \n        # Set secure file permissions (read/write for owner only)\n        os.chmod(target_path, 0o600)\n        \n        return f"Success: File uploaded as {unique_filename}"\n        \n    except Exception as e:\n        return f"Error: {str(e)}"\n\ndef is_pdf_file(content):\n    if len(content) < 4:\n        return False\n    return content[0:4] == b'%PDF'\n\ndef sanitize_filename(filename):\n    sanitized = re.sub(r'[^a-zA-Z0-9_\\-\\.]', '_', filename)\n    if len(sanitized) > 100:\n        sanitized = sanitized[:100]\n    return sanitized\n\ndef main():\n    # Test case 1: Valid PDF\n    valid_pdf = b'%PDF-1.4'\n    print("Test 1:", upload_pdf_file(valid_pdf, "test.pdf"))\n    \n    # Test case 2: Invalid extension\n    print("Test 2:", upload_pdf_file(valid_pdf, "test.txt"))\n    \n    # Test case 3: Empty file\n    print("Test 3:", upload_pdf_file(b'', "empty.pdf"))\n    \n    # Test case 4: Invalid PDF content\n    invalid_pdf = b'\\x00\\x01\\x02\\x03'
    print("Test 4:", upload_pdf_file(invalid_pdf, "fake.pdf"))
    
    # Test case 5: Malicious filename
    print("Test 5:", upload_pdf_file(valid_pdf, "../../../etc/passwd.pdf"))

if __name__ == "__main__":
    main()
