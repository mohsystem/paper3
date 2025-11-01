
import os
import stat

def create_secure_config_file(filename):
    try:
        # Create file with restrictive permissions (0o600)
        # O_CREAT: create if doesn't exist\n        # O_EXCL: fail if exists (prevents race conditions)\n        # O_WRONLY: write only\n        fd = os.open(filename, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)\n        os.close(fd)\n        return True\n    except FileExistsError:\n        print(f"File {filename} already exists")\n        return False\n    except OSError as e:\n        print(f"Error creating file: {e}")\n        return False\n\nif __name__ == "__main__":\n    # Test case 1: Create file with default name\n    print(f"Test 1: {create_secure_config_file('important_config')}")\n    \n    # Test case 2: Create file with different name\n    print(f"Test 2: {create_secure_config_file('important_config_2')}")\n    \n    # Test case 3: Try to create existing file\n    print(f"Test 3: {create_secure_config_file('important_config')}")\n    \n    # Test case 4: Create file in current directory\n    print(f"Test 4: {create_secure_config_file('./config_test')}")\n    \n    # Test case 5: Create file with specific path\n    print(f"Test 5: {create_secure_config_file('important_config_final')}")
