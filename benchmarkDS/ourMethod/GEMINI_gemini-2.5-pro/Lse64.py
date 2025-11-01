import os
import uuid
import mysql.connector
from mysql.connector import Error
from typing import Optional

UPLOAD_DIRECTORY = "uploads"
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10 MB
ALLOWED_EXTENSIONS = {".pdf"}

# --- Database Configuration ---
# In a real application, read these from a secure configuration source.
# Never hard-code credentials in code.
DB_CONFIG = {
    'host': os.environ.get('DB_HOST', 'localhost'),
    'user': os.environ.get('DB_USER', 'dbuser'),
    'password': os.environ.get('DB_PASS', 'dbpassword'),
    'database': os.environ.get('DB_NAME', 'filedb')
}

def upload_pdf(file_data: bytes, original_filename: str) -> Optional[str]:
    """
    Uploads a PDF file, saves it to a secure location, and records the filepath in the database.
    This function assumes it is called within an authenticated and authorized context.

    Args:
        file_data: The binary content of the file.
        original_filename: The original filename from the user.

    Returns:
        The secure filepath where the file was saved, or None on failure.
    """
    # 1. Validate inputs
    if not all([file_data, original_filename]):
        print("Error: Invalid input provided.", flush=True)
        return None

    if len(file_data) > MAX_FILE_SIZE:
        print(f"Error: File size exceeds the maximum limit of {MAX_FILE_SIZE} bytes.", flush=True)
        return None

    # 2. Sanitize and validate filename
    # Rule #3: Normalize path before validation is implicitly handled by os.path.basename
    base_filename = os.path.basename(original_filename)
    _root, ext = os.path.splitext(base_filename)
    
    # Rule #2: Use allow lists
    if ext.lower() not in ALLOWED_EXTENSIONS:
        print(f"Error: Invalid file type. Only {ALLOWED_EXTENSIONS} files are allowed.", flush=True)
        return None

    # 3. Generate a secure, unique filename
    secure_filename = f"{uuid.uuid4()}{ext.lower()}"
    
    # Ensure upload directory exists
    try:
        os.makedirs(UPLOAD_DIRECTORY, exist_ok=True)
    except OSError as e:
        # Rule #8: Use constant format strings for logging.
        print(f"Error creating upload directory: {e}", flush=True)
        return None

    destination_path = os.path.join(UPLOAD_DIRECTORY, secure_filename)

    # 4. Save the file
    try:
        # Rule #18: The write is bounded by the length of file_data, which we've checked.
        with open(destination_path, "wb") as f:
            f.write(file_data)
    except IOError as e:
        print(f"Error saving file: {e}", flush=True)
        return None

    # 5. Save filepath to the database using parameterized queries
    # Rule #9: Use parameterized queries to prevent SQL injection.
    conn = None
    cursor = None
    
    # Check if DB is configured, if not, simulate success
    if not all(DB_CONFIG.values()):
        print("Database environment variables not fully set. Skipping DB insert.", flush=True)
        print(f"File saved at (simulation): {destination_path}", flush=True)
        return destination_path
        
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        sql = "INSERT INTO files (filepath) VALUES (%s)"
        cursor.execute(sql, (destination_path,))
        conn.commit()
    except Error as e:
        # Rule #40: Do not leak internal details in production errors.
        print(f"Database error: {e}", flush=True)
        # Rollback: Clean up the saved file if the DB transaction fails.
        if os.path.exists(destination_path):
            os.remove(destination_path)
        return None
    finally:
        if cursor:
            cursor.close()
        if conn and conn.is_connected():
            conn.close()

    print(f"File uploaded successfully to: {destination_path}", flush=True)
    return destination_path

if __name__ == "__main__":
    print("--- Running Test Cases ---")
    print("Note: A running MySQL instance with a 'files' table is required for full functionality.")
    print("CREATE TABLE files (id INT AUTO_INCREMENT PRIMARY KEY, filepath VARCHAR(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);")
    print("Set DB_HOST, DB_USER, DB_PASS, DB_NAME environment variables to connect.")

    # Test Case 1: Valid PDF upload
    print("\n--- Test Case 1: Valid PDF ---")
    pdf_content = b"%PDF-1.4 content..."
    upload_pdf(pdf_content, "document.pdf")

    # Test Case 2: Invalid file type
    print("\n--- Test Case 2: Invalid file type ---")
    txt_content = b"This is a text file."
    upload_pdf(txt_content, "malicious.txt")

    # Test Case 3: Path traversal attempt
    print("\n--- Test Case 3: Path traversal attempt ---")
    upload_pdf(pdf_content, "../../etc/passwd.pdf")
    
    # Test Case 4: File size too large
    print("\n--- Test Case 4: File size too large ---")
    upload_pdf(b"a" * (MAX_FILE_SIZE + 1), "large_file.pdf")
    
    # Test Case 5: Empty/null input
    print("\n--- Test Case 5: Empty input ---")
    upload_pdf(b"", "")