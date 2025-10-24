
import urllib.request
import urllib.error
import re

class Task129:
    
    @staticmethod
    def retrieve_file_from_server(server_url, file_name):
        try:
            # Sanitize filename to prevent path traversal attacks
            sanitized_file_name = Task129.sanitize_file_name(file_name)
            
            # Construct full URL
            full_url = f"{server_url}/{sanitized_file_name}"
            
            # Set timeout for connection
            response = urllib.request.urlopen(full_url, timeout=5)
            
            # Read content
            content = response.read().decode('utf-8')
            
            return content
            
        except urllib.error.HTTPError as e:
            return f"Error: HTTP {e.code}"
        except urllib.error.URLError as e:
            return f"Error: {str(e.reason)}"
        except Exception as e:
            return f"Error: {str(e)}"
    
    @staticmethod
    def sanitize_file_name(file_name):
        # Remove path traversal sequences and dangerous characters
        if not file_name:
            return "default.txt"
        
        # Remove directory traversal attempts
        file_name = file_name.replace("..", "")
        file_name = file_name.replace("/", "")
        file_name = file_name.replace("\\\\", "")\n        \n        # Keep only alphanumeric, dots, hyphens, and underscores\n        file_name = re.sub(r'[^a-zA-Z0-9._-]', '', file_name)\n        \n        return file_name\n\n\ndef main():\n    # Test cases\n    server_url = "http://example.com/files"\n    \n    print("Test Case 1: Valid file name")\n    print(Task129.retrieve_file_from_server(server_url, "document.txt"))\n    print()\n    \n    print("Test Case 2: File with numbers")\n    print(Task129.retrieve_file_from_server(server_url, "report_2024.pdf"))\n    print()\n    \n    print("Test Case 3: Attempting path traversal (sanitized)")\n    print(Task129.retrieve_file_from_server(server_url, "../../../etc/passwd"))\n    print()\n    \n    print("Test Case 4: File with special characters")\n    print(Task129.retrieve_file_from_server(server_url, "data-file_01.json"))\n    print()\n    \n    print("Test Case 5: Empty filename")\n    print(Task129.retrieve_file_from_server(server_url, ""))\n    print()\n\n\nif __name__ == "__main__":
    main()
