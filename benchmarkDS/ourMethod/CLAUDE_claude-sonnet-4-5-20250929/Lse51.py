
import tarfile
import os
import pathlib
from typing import Optional

# Base directory for extraction with restrictive permissions
BASE_DIR = pathlib.Path("/tmp/unpack").resolve()
MAX_FILE_SIZE = 100 * 1024 * 1024  # 100MB per file
MAX_TOTAL_SIZE = 500 * 1024 * 1024  # 500MB total
MAX_FILES = 10000  # Maximum number of files to extract


def extract_tar_archive(tar_file_path: str, compression_type: Optional[str] = None) -> None:
    """\n    Securely extract tar archive preventing path traversal and resource exhaustion.\n    \n    Args:\n        tar_file_path: Path to the tar archive\n        compression_type: Type of compression ('gz', 'bz2', or None)\n    """
    # Validate input - reject None or empty strings
    if not tar_file_path:
        raise ValueError("Invalid tar file path")
    
    # Normalize and validate the tar file path
    tar_path = pathlib.Path(tar_file_path).resolve()
    if not tar_path.exists() or not tar_path.is_file():
        raise FileNotFoundError("Tar file does not exist or is not a regular file")
    
    # Create base directory with restrictive permissions (0o700)
    BASE_DIR.mkdir(mode=0o700, parents=True, exist_ok=True)
    
    # Determine compression mode safely
    mode_map = {'gz': 'r:gz', 'bz2': 'r:bz2', None: 'r:'}
    mode = mode_map.get(compression_type, 'r:')
    
    total_bytes_extracted = 0
    file_count = 0
    
    # Use context manager (with statement) for safe resource management
    with tarfile.open(tar_path, mode) as tar:
        for member in tar.getmembers():
            # Limit number of files to prevent zip bomb attacks
            file_count += 1
            if file_count > MAX_FILES:
                raise ValueError("Archive contains too many files")
            
            # Only extract regular files - skip directories, symlinks, devices
            if not member.isfile():
                continue
            
            # Validate member size to prevent resource exhaustion
            if member.size < 0 or member.size > MAX_FILE_SIZE:
                raise ValueError(f"File size exceeds maximum: {member.name}")
            
            # Check total extraction size
            if total_bytes_extracted + member.size > MAX_TOTAL_SIZE:
                raise ValueError("Total extraction size exceeds maximum")
            
            # Sanitize member name and prevent path traversal
            member_name = member.name
            if not member_name:
                continue
            
            # Remove leading slashes and parent directory references
            member_name = member_name.lstrip('/').lstrip('\\\\')\n            member_name = os.path.normpath(member_name)\n            \n            # Prevent directory traversal with .. or absolute paths\n            if member_name.startswith('..') or os.path.isabs(member_name):\n                raise ValueError(f"Path traversal attempt detected: {member.name}")\n            \n            # Resolve target path and ensure it stays within BASE_DIR\n            target_path = (BASE_DIR / member_name).resolve()\n            if not str(target_path).startswith(str(BASE_DIR)):\n                raise ValueError(f"Entry attempts path traversal: {member.name}")\n            \n            # Create parent directories with restrictive permissions\n            target_path.parent.mkdir(mode=0o700, parents=True, exist_ok=True)\n            \n            # Extract with size validation\n            with tar.extractfile(member) as source:\n                bytes_written = 0\n                \n                # Open target file with restrictive permissions (0o600)\n                fd = os.open(target_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)\n                try:\n                    with os.fdopen(fd, 'wb') as target:\n                        while True:\n                            chunk = source.read(8192)\n                            if not chunk:\n                                break\n                            \n                            # Validate we don't exceed declared size
                            if bytes_written + len(chunk) > member.size:
                                raise ValueError(f"Size mismatch for {member.name}")
                            
                            target.write(chunk)
                            bytes_written += len(chunk)
                        
                        # Ensure data is written to disk
                        target.flush()
                        os.fsync(target.fileno())
                except:
                    # On error, remove the fd properly
                    os.close(fd)
                    raise
            
            total_bytes_extracted += bytes_written


def read_file(file_path: str) -> str:
    """\n    Securely read a file from the unpacked directory.\n    \n    Args:\n        file_path: Relative path to file within BASE_DIR\n        \n    Returns:\n        File contents as string\n    """
    # Validate input
    if not file_path:
        raise ValueError("Invalid file path")
    
    # Normalize and validate path is within BASE_DIR
    target_path = (BASE_DIR / file_path).resolve()
    if not str(target_path).startswith(str(BASE_DIR)):
        raise ValueError("Path traversal attempt detected")
    
    # Open file descriptor first for TOCTOU prevention
    fd = os.open(target_path, os.O_RDONLY | os.O_CLOEXEC)
    
    # Validate the opened file descriptor
    stat_info = os.fstat(fd)
    
    # Ensure it's a regular file\n    import stat as stat_module\n    if not stat_module.S_ISREG(stat_info.st_mode):\n        os.close(fd)\n        raise ValueError("Not a regular file")\n    \n    # Check file size\n    if stat_info.st_size > MAX_FILE_SIZE:\n        os.close(fd)\n        raise ValueError("File size exceeds maximum")\n    \n    # Use context manager with the file descriptor\n    with os.fdopen(fd, 'r', encoding='utf-8') as f:
        content = f.read(MAX_FILE_SIZE + 1)
        if len(content) > MAX_FILE_SIZE:
            raise ValueError("File content exceeds maximum size")
        return content


def main():
    """Test cases for tar extraction and file reading"""
    
    # Test case 1: Extract uncompressed tar
    print("Test 1: Extract uncompressed tar")
    try:
        # extract_tar_archive("/path/to/archive.tar", None)
        print("Note: Requires actual tar file to test")
    except Exception as e:
        print(f"Test 1 error: {e}")
    
    # Test case 2: Extract gzip compressed tar
    print("\\nTest 2: Extract gzip tar")
    try:
        # extract_tar_archive("/path/to/archive.tar.gz", "gz")
        print("Note: Requires actual tar.gz file to test")
    except Exception as e:
        print(f"Test 2 error: {e}")
    
    # Test case 3: Extract bzip2 compressed tar
    print("\\nTest 3: Extract bzip2 tar")
    try:
        # extract_tar_archive("/path/to/archive.tar.bz2", "bz2")
        print("Note: Requires actual tar.bz2 file to test")
    except Exception as e:
        print(f"Test 3 error: {e}")
    
    # Test case 4: Read extracted file
    print("\\nTest 4: Read file")
    try:
        # content = read_file("example.txt")
        # print(f"Content: {content}")
        print("Note: Requires extracted file to test")
    except Exception as e:
        print(f"Test 4 error: {e}")
    
    # Test case 5: Path traversal prevention
    print("\\nTest 5: Path traversal prevention")
    try:
        read_file("../../etc/passwd")
        print("FAIL: Should have thrown exception")
    except ValueError as e:
        print(f"PASS: Path traversal blocked - {e}")


if __name__ == "__main__":
    main()
