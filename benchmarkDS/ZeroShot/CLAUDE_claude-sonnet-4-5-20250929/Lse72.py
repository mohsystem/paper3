
import os
import re
import time
from pathlib import Path

class Lse72:
    MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB
    ALLOWED_EXTENSIONS = {'txt', 'pdf', 'jpg', 'jpeg', 'png', 'doc', 'docx'}
    UPLOAD_DIR = 'uploads'
    
    @staticmethod
    def upload_file(file_content, filename):
        try:
            # Validate file size
            if len(file_content) > Lse72.MAX_FILE_SIZE:
                return "Error: File size exceeds maximum allowed size"
            
            # Sanitize filename
            sanitized_filename = Lse72.sanitize_filename(filename)
            if sanitized_filename is None:
                return "Error: Invalid filename"
            
            # Validate file extension
            if not Lse72.is_allowed_extension(sanitized_filename):
                return "Error: File type not allowed"
            
            # Create uploads directory if it doesn't exist\n            Path(Lse72.UPLOAD_DIR).mkdir(exist_ok=True)\n            \n            # Generate unique filename\n            unique_filename = Lse72.generate_unique_filename(sanitized_filename)\n            file_path = os.path.join(Lse72.UPLOAD_DIR, unique_filename)\n            \n            # Save file\n            with open(file_path, 'wb') as f:\n                if isinstance(file_content, str):\n                    f.write(file_content.encode())\n                else:\n                    f.write(file_content)\n            \n            return f"File uploaded successfully: {unique_filename}"\n        except Exception as e:\n            return f"Error: {str(e)}"\n    \n    @staticmethod\n    def sanitize_filename(filename):\n        if not filename:\n            return None\n        \n        # Remove path traversal attempts\n        filename = filename.replace('..', '')\n        filename = filename.replace('/', '').replace('\\\\', '')\n        \n        # Allow only alphanumeric, dots, dashes, and underscores\n        if not re.match(r'^[a-zA-Z0-9._-]+$', filename):\n            return None\n        \n        return filename\n    \n    @staticmethod\n    def is_allowed_extension(filename):\n        if '.' not in filename:\n            return False\n        extension = filename.rsplit('.', 1)[1].lower()\n        return extension in Lse72.ALLOWED_EXTENSIONS\n    \n    @staticmethod\n    def generate_unique_filename(filename):\n        timestamp = str(int(time.time() * 1000))\n        if '.' in filename:\n            name, extension = filename.rsplit('.', 1)\n            return f"{name}_{timestamp}.{extension}"\n        return f"{filename}_{timestamp}"\n\n\nif __name__ == "__main__":\n    # Test Case 1: Valid text file\n    print(Lse72.upload_file("This is a test file content", "test1.txt"))\n    \n    # Test Case 2: Valid PDF file\n    print(Lse72.upload_file(b'\\x00' * 1024, "document.pdf"))\n    \n    # Test Case 3: Invalid extension\n    print(Lse72.upload_file("malicious content", "malicious.exe"))\n    \n    # Test Case 4: Path traversal attempt\n    print(Lse72.upload_file("hacker content", "../../../etc/passwd"))\n    \n    # Test Case 5: Valid image file\n    print(Lse72.upload_file(b'\\x00' * 2048, "photo.jpg"))
