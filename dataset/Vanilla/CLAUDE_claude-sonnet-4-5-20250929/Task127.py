
import os
import sys
import time

def read_and_write_file(source_file_path, temp_directory):
    reader = None
    writer = None
    temp_file_path = None
    
    try:
        # Validate input parameters
        if source_file_path is None or not source_file_path.strip():
            raise ValueError("Source file path cannot be None or empty")
        if temp_directory is None or not temp_directory.strip():
            raise ValueError("Temp directory cannot be None or empty")
        
        # Check if source file exists
        if not os.path.exists(source_file_path):
            raise FileNotFoundError(f"Source file does not exist: {source_file_path}")
        if not os.path.isfile(source_file_path):
            raise ValueError(f"Source path is not a file: {source_file_path}")
        if not os.access(source_file_path, os.R_OK):
            raise IOError(f"Cannot read source file: {source_file_path}")
        
        # Create temp directory if it doesn't exist\n        if not os.path.exists(temp_directory):\n            try:\n                os.makedirs(temp_directory)\n            except OSError as e:\n                raise IOError(f"Failed to create temp directory: {temp_directory}") from e\n        \n        # Create temp file\n        file_name = os.path.basename(source_file_path)\n        temp_file_path = os.path.join(temp_directory, f"temp_{int(time.time() * 1000)}_{file_name}")\n        \n        # Read from source and write to temp\n        with open(source_file_path, 'r', encoding='utf-8') as reader:\n            with open(temp_file_path, 'w', encoding='utf-8') as writer:\n                for line in reader:\n                    writer.write(line)\n        \n        return temp_file_path\n        \n    except FileNotFoundError as e:\n        print(f"File not found error: {e}", file=sys.stderr)\n        return None\n    except IOError as e:\n        print(f"IO error: {e}", file=sys.stderr)\n        return None\n    except ValueError as e:\n        print(f"Invalid argument: {e}", file=sys.stderr)\n        return None\n    except Exception as e:\n        print(f"Unexpected error: {e}", file=sys.stderr)\n        return None\n\ndef main():\n    print("Testing File Read and Write with Exception Handling\\n")\n    \n    # Test Case 1: Valid file and temp directory\n    print("Test Case 1: Valid file and temp directory")\n    try:\n        with open("test1.txt", 'w') as f:\n            f.write("Hello World!\\nThis is test file 1.")\n        \n        result = read_and_write_file("test1.txt", "./temp")\n        print(f"Result: {'Success - ' + result if result else 'Failed'}")\n        if os.path.exists("test1.txt"):\n            os.remove("test1.txt")\n    except Exception as e:\n        print(f"Setup error: {e}")\n    print()\n    \n    # Test Case 2: Non-existent source file\n    print("Test Case 2: Non-existent source file")\n    result2 = read_and_write_file("nonexistent.txt", "./temp")\n    print(f"Result: {'Success - ' + result2 if result2 else 'Failed (Expected)'}")\n    print()\n    \n    # Test Case 3: None source file path\n    print("Test Case 3: None source file path")\n    result3 = read_and_write_file(None, "./temp")\n    print(f"Result: {'Success - ' + result3 if result3 else 'Failed (Expected)'}")\n    print()\n    \n    # Test Case 4: Empty source file path\n    print("Test Case 4: Empty source file path")\n    result4 = read_and_write_file("", "./temp")\n    print(f"Result: {'Success - ' + result4 if result4 else 'Failed (Expected)'}")\n    print()\n    \n    # Test Case 5: Valid file with special characters\n    print("Test Case 5: Valid file with special characters")\n    try:\n        with open("test5.txt", 'w', encoding='utf-8') as f:\n            f.write("Special chars: @#$%^&*()\\nUnicode: \\u00A9 \\u00AE")\n        \n        result5 = read_and_write_file("test5.txt", "./temp")\n        print(f"Result: {'Success - ' + result5 if result5 else 'Failed'}")\n        if os.path.exists("test5.txt"):\n            os.remove("test5.txt")\n    except Exception as e:\n        print(f"Setup error: {e}")\n    print()\n    \n    # Cleanup\n    try:\n        if os.path.exists("./temp"):\n            for file in os.listdir("./temp"):\n                os.remove(os.path.join("./temp", file))\n            os.rmdir("./temp")\n    except Exception as e:\n        print(f"Cleanup error: {e}", file=sys.stderr)\n\nif __name__ == "__main__":
    main()
