
import os
import sys

def read_file_from_argument(filename):
    if filename is None or not filename.strip():
        return "Error: Filename is null or empty"
    
    # Validate filename format
    if '\\0' in filename or len(filename) > 255:
        return "Error: Invalid filename format"
    
    # Check if file exists
    if not os.path.exists(filename):
        return f"Error: File does not exist: {filename}"
    
    # Check if it's a file (not a directory)\n    if not os.path.isfile(filename):\n        return f"Error: Path is not a file: {filename}"\n    \n    # Check if file is readable\n    if not os.access(filename, os.R_OK):\n        return f"Error: File is not readable: {filename}"\n    \n    try:\n        # Read file content\n        with open(filename, 'r', encoding='utf-8') as file:
            content = file.read()
        return content
    except IOError as e:
        return f"Error: Failed to read file: {str(e)}"
    except Exception as e:
        return f"Error: Unexpected error: {str(e)}"

def main():
    print("=== Test Case 1: Valid file ===")
    try:
        with open("test1.txt", "w") as f:
            f.write("Hello World!\\nThis is a test file.")
        result1 = read_file_from_argument("test1.txt")
        print(result1)
    except IOError as e:
        print(f"Setup error: {e}")
    
    print("\\n=== Test Case 2: Non-existent file ===")
    result2 = read_file_from_argument("nonexistent.txt")
    print(result2)
    
    print("\\n=== Test Case 3: Null filename ===")
    result3 = read_file_from_argument(None)
    print(result3)
    
    print("\\n=== Test Case 4: Empty filename ===")
    result4 = read_file_from_argument("")
    print(result4)
    
    print("\\n=== Test Case 5: Another valid file ===")
    try:
        with open("test2.txt", "w") as f:
            f.write("Line 1\\nLine 2\\nLine 3")
        result5 = read_file_from_argument("test2.txt")
        print(result5)
    except IOError as e:
        print(f"Setup error: {e}")
    
    # Cleanup
    try:
        if os.path.exists("test1.txt"):
            os.remove("test1.txt")
        if os.path.exists("test2.txt"):
            os.remove("test2.txt")
    except:
        pass

if __name__ == "__main__":
    main()
