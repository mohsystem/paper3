
import re

class Task125:
    ALLOWED_RESOURCES = {
        "document1.txt", "document2.txt", "image1.jpg", "image2.jpg", "data.csv"
    }
    
    VALID_RESOURCE_PATTERN = re.compile(r'^[a-zA-Z0-9._-]+$')
    
    @staticmethod
    def access_resource(username, resource_name):
        """\n        Securely access resources based on user input with validation and whitelist checking.\n        """
        # Input validation
        if not username or not username.strip():
            return "Error: Invalid username"
        
        if not resource_name or not resource_name.strip():
            return "Error: Invalid resource name"
        
        # Sanitize input - remove whitespace
        resource_name = resource_name.strip()
        
        # Validate resource name format (prevent path traversal)
        if not Task125.VALID_RESOURCE_PATTERN.match(resource_name):
            return "Error: Invalid resource name format"
        
        # Check for path traversal attempts
        if ".." in resource_name or "/" in resource_name or "\\\\" in resource_name:\n            return "Error: Path traversal attempt detected"\n        \n        # Check if resource is in whitelist\n        if resource_name not in Task125.ALLOWED_RESOURCES:\n            return "Error: Access denied - resource not found or unauthorized"\n        \n        # Simulate resource access\n        return f"Success: User '{username}' accessed resource '{resource_name}'"\n\n\ndef main():\n    print("=== Resource Access Control System ===\
")\n    \n    # Test case 1: Valid access\n    print("Test 1 - Valid access:")\n    print(Task125.access_resource("john_doe", "document1.txt"))\n    print()\n    \n    # Test case 2: Valid access to different resource\n    print("Test 2 - Valid access to image:")\n    print(Task125.access_resource("jane_smith", "image1.jpg"))\n    print()\n    \n    # Test case 3: Attempt to access non-existent resource\n    print("Test 3 - Non-existent resource:")\n    print(Task125.access_resource("user1", "secret.txt"))\n    print()\n    \n    # Test case 4: Path traversal attempt\n    print("Test 4 - Path traversal attempt:")\n    print(Task125.access_resource("attacker", "../../../etc/passwd"))\n    print()\n    \n    # Test case 5: Invalid input\n    print("Test 5 - Invalid input:")\n    print(Task125.access_resource("", "document1.txt"))\n    print(Task125.access_resource("user2", ""))\n    print()\n\n\nif __name__ == "__main__":
    main()
