
import os
import hashlib
import mysql.connector
from mysql.connector import Error
from typing import Optional

class Lse67:
    MAX_FILE_SIZE = 5 * 1024 * 1024  # 5MB
    ALLOWED_EXTENSIONS = ['jpg', 'jpeg', 'png', 'gif']
    
    @staticmethod
    def store_image(image_path: str, db_host: str, db_user: str, db_password: str, db_name: str) -> bool:
        """\n        Securely store an image file in MySQL database\n        """
        conn = None
        cursor = None
        
        try:
            # Validate file exists
            if not os.path.isfile(image_path):
                print("Error: File does not exist or is not a file")
                return False
            
            # Check file size
            file_size = os.path.getsize(image_path)
            if file_size > Lse67.MAX_FILE_SIZE:
                print("Error: File size exceeds maximum allowed size")
                return False
            
            # Validate file extension
            file_name = os.path.basename(image_path)
            extension = file_name.split('.')[-1].lower()
            if extension not in Lse67.ALLOWED_EXTENSIONS:
                print("Error: Invalid file extension")
                return False
            
            # Read file content
            with open(image_path, 'rb') as f:
                image_data = f.read()
            
            # Verify it's actually an image by checking magic bytes\n            if not Lse67.is_valid_image_file(image_data):\n                print("Error: File is not a valid image")\n                return False\n            \n            # Calculate file hash for integrity\n            file_hash = Lse67.calculate_sha256(image_data)\n            \n            # Connect to database\n            conn = mysql.connector.connect(\n                host=db_host,\n                user=db_user,\n                password=db_password,\n                database=db_name\n            )\n            \n            if not conn.is_connected():\n                print("Error: Could not connect to database")\n                return False\n            \n            cursor = conn.cursor()\n            \n            # Create table if not exists\n            Lse67.create_table_if_not_exists(cursor)\n            \n            # Insert image into database\n            sql = """INSERT INTO images \n                     (file_name, file_size, file_hash, image_data, upload_date) \n                     VALUES (%s, %s, %s, %s, NOW())"""\n            cursor.execute(sql, (file_name, file_size, file_hash, image_data))\n            conn.commit()\n            \n            return cursor.rowcount > 0\n            \n        except Error as e:\n            print(f"Database Error: {e}")\n            return False\n        except Exception as e:\n            print(f"Error: {e}")\n            return False\n        finally:\n            if cursor:\n                cursor.close()\n            if conn and conn.is_connected():\n                conn.close()\n    \n    @staticmethod\n    def create_table_if_not_exists(cursor):\n        """Create images table if it doesn't exist"""\n        sql = """CREATE TABLE IF NOT EXISTS images (
                 id INT AUTO_INCREMENT PRIMARY KEY,
                 file_name VARCHAR(255) NOT NULL,
                 file_size BIGINT NOT NULL,
                 file_hash VARCHAR(64) NOT NULL,
                 image_data LONGBLOB NOT NULL,
                 upload_date DATETIME NOT NULL
                 )"""\n        cursor.execute(sql)\n    \n    @staticmethod\n    def is_valid_image_file(data: bytes) -> bool:\n        """Validate image by checking magic bytes"""\n        if len(data) < 4:\n            return False\n        \n        # JPEG: FF D8 FF\n        if data[0:3] == b'\\xFF\\xD8\\xFF':\n            return True\n        # PNG: 89 50 4E 47\n        if data[0:4] == b'\\x89PNG':\n            return True\n        # GIF: 47 49 46 38\n        if data[0:4] == b'GIF8':\n            return True\n        \n        return False\n    \n    @staticmethod\n    def calculate_sha256(data: bytes) -> str:\n        """Calculate SHA-256 hash of data"""\n        sha256_hash = hashlib.sha256()\n        sha256_hash.update(data)\n        return sha256_hash.hexdigest()\n\n\ndef main():\n    """Main function with test cases"""\n    # Database configuration - CHANGE THESE VALUES\n    db_host = "localhost"\n    db_user = "root"\n    db_password = "password"\n    db_name = "imagedb"\n    \n    # Test cases\n    print("Test Case 1: Valid JPEG image")\n    result1 = Lse67.store_image("test1.jpg", db_host, db_user, db_password, db_name)\n    print(f"Result: {result1}\
")\n    \n    print("Test Case 2: Valid PNG image")\n    result2 = Lse67.store_image("test2.png", db_host, db_user, db_password, db_name)\n    print(f"Result: {result2}\
")\n    \n    print("Test Case 3: Non-existent file")\n    result3 = Lse67.store_image("nonexistent.jpg", db_host, db_user, db_password, db_name)\n    print(f"Result: {result3}\
")\n    \n    print("Test Case 4: Invalid extension")\n    result4 = Lse67.store_image("test.txt", db_host, db_user, db_password, db_name)\n    print(f"Result: {result4}\
")\n    \n    print("Test Case 5: Valid GIF image")\n    result5 = Lse67.store_image("test5.gif", db_host, db_user, db_password, db_name)\n    print(f"Result: {result5}")\n\n\nif __name__ == "__main__":
    main()
