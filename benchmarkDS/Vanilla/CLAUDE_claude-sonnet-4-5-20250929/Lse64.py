
import os
import mysql.connector
from datetime import datetime
import shutil

UPLOAD_DIR = "uploads/"
DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': 'password',
    'database': 'pdf_db'
}

def upload_pdf(source_file_path, file_name):
    """Upload PDF function"""
    try:
        # Create uploads directory if it doesn't exist\n        if not os.path.exists(UPLOAD_DIR):\n            os.makedirs(UPLOAD_DIR)\n        \n        # Generate unique filename\n        unique_filename = f"{int(datetime.now().timestamp() * 1000)}_{file_name}"\n        destination_path = os.path.join(UPLOAD_DIR, unique_filename)\n        \n        # Copy file to uploads folder\n        shutil.copy2(source_file_path, destination_path)\n        \n        # Save filepath to database\n        save_to_database(destination_path, file_name)\n        \n        return destination_path\n    except Exception as e:\n        print(f"Error uploading PDF: {e}")\n        return None\n\ndef save_to_database(filepath, filename):\n    """Save filepath to database"""\n    try:\n        conn = mysql.connector.connect(**DB_CONFIG)\n        cursor = conn.cursor()\n        \n        sql = "INSERT INTO pdf_files (filepath, filename, upload_date) VALUES (%s, %s, NOW())"\n        cursor.execute(sql, (filepath, filename))\n        \n        conn.commit()\n        cursor.close()\n        conn.close()\n    except Exception as e:\n        print(f"Error saving to database: {e}")\n\ndef download_pdf(file_id, download_path):\n    """Download PDF function"""\n    try:\n        conn = mysql.connector.connect(**DB_CONFIG)\n        cursor = conn.cursor()\n        \n        sql = "SELECT filepath FROM pdf_files WHERE id = %s"\n        cursor.execute(sql, (file_id,))\n        result = cursor.fetchone()\n        \n        if result:\n            filepath = result[0]\n            shutil.copy2(filepath, download_path)\n            cursor.close()\n            conn.close()\n            return True\n        \n        cursor.close()\n        conn.close()\n    except Exception as e:\n        print(f"Error downloading PDF: {e}")\n    \n    return False\n\nif __name__ == "__main__":\n    print("PDF Upload/Download System Test Cases:")\n    \n    # Test Case 1: Upload a PDF file\n    print("\\nTest Case 1: Upload PDF")\n    result1 = upload_pdf("test1.pdf", "document1.pdf")\n    print(f"Upload result: {'Success - ' + result1 if result1 else 'Failed'}")\n    \n    # Test Case 2: Upload another PDF file\n    print("\\nTest Case 2: Upload another PDF")\n    result2 = upload_pdf("test2.pdf", "document2.pdf")\n    print(f"Upload result: {'Success - ' + result2 if result2 else 'Failed'}")\n    \n    # Test Case 3: Upload PDF with special characters\n    print("\\nTest Case 3: Upload PDF with special name")\n    result3 = upload_pdf("test3.pdf", "my_document_2024.pdf")\n    print(f"Upload result: {'Success - ' + result3 if result3 else 'Failed'}")\n    \n    # Test Case 4: Download PDF by ID\n    print("\\nTest Case 4: Download PDF")\n    download1 = download_pdf(1, "downloaded_file1.pdf")\n    print(f"Download result: {'Success' if download1 else 'Failed'}")\n    \n    # Test Case 5: Download non-existent PDF\n    print("\\nTest Case 5: Download non-existent PDF")\n    download2 = download_pdf(9999, "downloaded_file2.pdf")\n    print(f"Download result: {'Success' if download2 else 'Failed (Expected)'}")
