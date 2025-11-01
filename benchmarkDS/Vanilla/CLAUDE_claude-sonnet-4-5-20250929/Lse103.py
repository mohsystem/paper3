
import os

def create_file_excl(filename):
    """Creates file with O_CREAT | O_TRUNC | O_WRONLY | O_EXCL flags"""
    try:
        # O_CREAT: create if doesn't exist\n        # O_EXCL: fail if exists\n        # O_WRONLY: write only\n        # O_TRUNC: truncate to 0 bytes\n        fd = os.open(filename, os.O_CREAT | os.O_EXCL | os.O_WRONLY | os.O_TRUNC, 0o666)\n        os.close(fd)\n        return 0\n    except OSError:\n        return -1\n\ndef create_file_trunc(filename):\n    """Creates file with O_CREAT | O_TRUNC | O_WRONLY flags (without O_EXCL)"""\n    try:\n        fd = os.open(filename, os.O_CREAT | os.O_WRONLY | os.O_TRUNC, 0o666)\n        os.close(fd)\n        return 0\n    except OSError:\n        return -1\n\nif __name__ == "__main__":\n    # Test case 1: Create new file with O_EXCL\n    print(f"Test 1 - Create new file: {create_file_excl('test1.txt')}")\n    \n    # Test case 2: Try to create existing file with O_EXCL (should fail)\n    print(f"Test 2 - Create existing file with O_EXCL: {create_file_excl('test1.txt')}")\n    \n    # Test case 3: Create/truncate file without O_EXCL\n    print(f"Test 3 - Create/truncate file: {create_file_trunc('test2.txt')}")\n    \n    # Test case 4: Truncate existing file\n    print(f"Test 4 - Truncate existing file: {create_file_trunc('test2.txt')}")\n    \n    # Test case 5: Create file in valid directory\n    print(f"Test 5 - Create another file: {create_file_excl('test3.txt')}")\n    \n    # Cleanup\n    try:\n        os.remove('test1.txt')\n        os.remove('test2.txt')\n        os.remove('test3.txt')
    except:
        pass
