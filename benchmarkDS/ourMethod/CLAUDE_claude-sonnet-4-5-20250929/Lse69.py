
import os
import base64
import tempfile
import secrets
from pathlib import Path

# Upload directory with restrictive permissions
UPLOAD_DIR = "/var/uploads"
MAX_FILE_SIZE = 5 * 1024 * 1024  # 5MB limit

def upload_image(image_data):
    """\n    Uploads and encodes an image file\n    \n    Args:\n        image_data: bytes containing the raw image data\n        \n    Returns:\n        str: filename where the encoded image was saved, or None on failure\n    """
    # Input validation: check for None and size limits
    if image_data is None or len(image_data) == 0:
        print("Invalid image data", file=__import__('sys').stderr)
        return None
    
    if len(image_data) > MAX_FILE_SIZE:
        print("File size exceeds limit", file=__import__('sys').stderr)
        return None
    
    try:
        # Create upload directory if it doesn't exist with restrictive permissions (700)\n        upload_path = Path(UPLOAD_DIR)\n        upload_path.mkdir(parents=True, exist_ok=True)\n        os.chmod(upload_path, 0o700)\n        \n        # Generate a safe, random filename to prevent path traversal\n        filename = f"image_{int(__import__('time').time() * 1000)}_{secrets.randbelow(100000)}.b64"\n        \n        # Normalize and validate the path resolves within UPLOAD_DIR\n        output_path = (upload_path / filename).resolve()\n        if not str(output_path).startswith(str(upload_path.resolve())):\n            print("Path traversal attempt detected", file=__import__('sys').stderr)\n            return None\n        \n        # Encode the image data using base64\n        encoded_image = base64.b64encode(image_data).decode('utf-8')\n        \n        # Open temp file with O_CREAT|O_EXCL to prevent race conditions\n        # Use restrictive permissions (0o600 - owner read/write only)\n        fd = os.open(\n            upload_path,\n            os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_CLOEXEC,\n            0o600\n        )\n        \n        # Create a temporary file in the same directory for atomic write\n        temp_fd, temp_path = tempfile.mkstemp(\n            dir=str(upload_path),\n            prefix="temp_",\n            suffix=".tmp"\n        )\n        os.close(fd)  # Close the validation fd\n        \n        # Set restrictive permissions on temp file\n        os.chmod(temp_path, 0o600)\n        \n        # Write encoded data to temp file\n        with os.fdopen(temp_fd, 'w', encoding='utf-8') as f:\n            f.write(encoded_image)\n            f.flush()\n            os.fsync(f.fileno())\n        \n        # Atomically move temp file to final destination\n        os.replace(temp_path, str(output_path))\n        \n        return filename\n        \n    except Exception as e:\n        print(f"Error processing image upload", file=__import__('sys').stderr)\n        return None\n\n\ndef main():\n    # Test case 1: Valid small image\n    test_image1 = b"Test image data 1"\n    result1 = upload_image(test_image1)\n    print(f"Test 1 - Valid small image: {'PASS' if result1 else 'FAIL'}")\n    \n    # Test case 2: None input\n    result2 = upload_image(None)\n    print(f"Test 2 - None input: {'PASS' if result2 is None else 'FAIL'}")\n    \n    # Test case 3: Empty bytes\n    test_image3 = b""\n    result3 = upload_image(test_image3)\n    print(f"Test 3 - Empty bytes: {'PASS' if result3 is None else 'FAIL'}")\n    \n    # Test case 4: Valid image with binary data\n    test_image4 = bytes(range(256)) * 4\n    result4 = upload_image(test_image4)\n    print(f"Test 4 - Binary data: {'PASS' if result4 else 'FAIL'}")\n    \n    # Test case 5: Image at size boundary\n    test_image5 = b"A" * 1024\n    result5 = upload_image(test_image5)\n    print(f"Test 5 - Size boundary: {'PASS' if result5 else 'FAIL'}")\n\n\nif __name__ == "__main__":
    main()
