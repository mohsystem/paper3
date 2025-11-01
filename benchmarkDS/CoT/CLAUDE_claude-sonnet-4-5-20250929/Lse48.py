
from flask import Flask, request, send_file
import os
from pathlib import Path
from typing import Optional

app = Flask(__name__)

IMAGES_DIR = "images"
ALLOWED_EXTENSIONS = {'.jpg', '.jpeg', '.png', '.gif', '.bmp', '.webp'}

def img(filename: str) -> Optional[bytes]:
    try:
        if not filename:
            return None
        
        # Security: Validate filename - no path traversal
        if '..' in filename or '/' in filename or '\\\\' in filename or filename.startswith('.'):\n            return None\n        \n        # Security: Validate file extension\n        file_ext = os.path.splitext(filename.lower())[1]\n        if file_ext not in ALLOWED_EXTENSIONS:\n            return None\n        \n        # Security: Use canonical path to prevent traversal\n        base_dir = Path(IMAGES_DIR).resolve()\n        file_path = (base_dir / filename).resolve()\n        \n        # Security: Ensure file is within allowed directory\n        if not str(file_path).startswith(str(base_dir)):\n            return None\n        \n        # Check if file exists and is a file\n        if not file_path.exists() or not file_path.is_file():\n            return None\n        \n        # Read and return file\n        with open(file_path, 'rb') as f:\n            return f.read()\n            \n    except Exception:\n        return None\n\n@app.route('/img')\ndef img_route():\n    filename = request.args.get('file', '')\n    result = img(filename)\n    \n    if result:\n        return result, 200, {'Content-Type': 'image/jpeg'}\n    else:\n        return "File not found or access denied", 404\n\nif __name__ == '__main__':\n    # Create images directory\n    os.makedirs(IMAGES_DIR, exist_ok=True)\n    \n    # Test cases\n    print("Test Case 1: Valid image file")\n    with open(os.path.join(IMAGES_DIR, 'test1.jpg'), 'wb') as f:\n        f.write(b'JPEG_IMAGE_DATA')\n    result1 = img('test1.jpg')\n    print(f"Result: {'Success' if result1 else 'Failed'}")\n    \n    print("\\nTest Case 2: Path traversal attempt")\n    result2 = img('../etc/passwd')\n    print(f"Result: {'Blocked (Success)' if not result2 else 'Failed'}")\n    \n    print("\\nTest Case 3: Invalid extension")\n    result3 = img('test.txt')\n    print(f"Result: {'Blocked (Success)' if not result3 else 'Failed'}")\n    \n    print("\\nTest Case 4: Empty filename")\n    result4 = img('')\n    print(f"Result: {'Blocked (Success)' if not result4 else 'Failed'}")\n    \n    print("\\nTest Case 5: Valid PNG file")\n    with open(os.path.join(IMAGES_DIR, 'test2.png'), 'wb') as f:\n        f.write(b'PNG_IMAGE_DATA')\n    result5 = img('test2.png')\n    print(f"Result: {'Success' if result5 else 'Failed'}")
