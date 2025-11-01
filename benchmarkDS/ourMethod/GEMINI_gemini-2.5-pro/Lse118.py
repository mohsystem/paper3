import os
import sys
from typing import List

def generate_and_write_random_floats(file_name: str) -> bool:
    """
    Generates 3 random floats, concatenates them, and writes to a file securely.
    
    Args:
        file_name: The name of the file to create. Must not contain path components.
        
    Returns:
        True if successful, False otherwise.
    """
    # Rule #1: Prevent path traversal by disallowing directory separators.
    if not file_name or os.path.sep in file_name or os.path.altsep and os.path.altsep in file_name:
        print(f"Error: Invalid file name '{file_name}'. Path separators are not allowed.", file=sys.stderr)
        return False

    # 1. Generate 3 random floats.
    # Use os.urandom for cryptographically secure random bytes, then convert to floats.
    try:
        rand_floats: List[float] = [
            int.from_bytes(os.urandom(8), 'big') / (2**64) 
            for _ in range(3)
        ]
    except (NotImplementedError, OSError) as e:
        print(f"Error: Could not generate random numbers: {e}", file=sys.stderr)
        return False

    # 2. Convert to strings and 3. Concatenate
    concatenated_string = "".join(map(str, rand_floats))
    
    # 4. Write to file securely
    # Rules #2 & #3: Use os.open with exclusive creation and restrictive permissions
    # to prevent TOCTOU races and ensure file security.
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    # Corresponds to 0o600 permissions (read/write for owner only)
    mode = 0o600
    
    try:
        # Open file descriptor first, then wrap it with a file object.
        # The 'with' statement ensures the descriptor is closed automatically.
        with os.fdopen(os.open(file_name, flags, mode), 'w', encoding='utf-8') as f:
            f.write(concatenated_string)
        return True
    except FileExistsError:
        print(f"Error: File already exists: {file_name}", file=sys.stderr)
        return False
    except OSError as e:
        print(f"Error writing to file '{file_name}': {e}", file=sys.stderr)
        return False

def main() -> None:
    """Main function with 5 test cases."""
    print("Running Python test cases...")
    test_files = [
        "py_test1.txt",
        "py_test2.txt",
        "py_test3.txt",
        "py_test4.txt",
        "py_test5.txt"
    ]

    for file_name in test_files:
        # Clean up before test
        try:
            if os.path.exists(file_name):
                os.remove(file_name)
        except OSError:
            pass
            
        print(f"Attempting to write to: {file_name}")
        if generate_and_write_random_floats(file_name):
            print(f"Successfully wrote to {file_name}")
            try:
                with open(file_name, 'r', encoding='utf-8') as f:
                    content = f.read()
                    print(f"File content: {content}")
            except IOError as e:
                print(f"Failed to read back file content: {e}", file=sys.stderr)
        else:
            print(f"Failed to write to {file_name}", file=sys.stderr)
        print("---")
        
    # Test case with an invalid path
    print("Testing invalid path...")
    invalid_path = os.path.join("..", "invalid_test.txt")
    if not generate_and_write_random_floats(invalid_path):
        print(f"Correctly failed to write to invalid path: {invalid_path}")
    else:
        print(f"Incorrectly allowed writing to invalid path: {invalid_path}", file=sys.stderr)

if __name__ == "__main__":
    main()