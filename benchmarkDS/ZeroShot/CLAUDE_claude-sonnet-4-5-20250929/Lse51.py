
import tarfile
import os

def extract_tar_archive(tar_file_path, dest_directory="/tmp/unpack"):
    """\n    Extracts a tar archive to the specified directory.\n    Supports .tar, .tar.gz, .tgz, and .tar.bz2 formats.\n    """
    # Create destination directory if it doesn't exist\n    os.makedirs(dest_directory, exist_ok=True)\n    \n    # Security check function to prevent path traversal\n    def is_safe_path(base_path, path):\n        return os.path.commonpath([base_path, path]) == base_path\n    \n    with tarfile.open(tar_file_path, 'r:*') as tar:\n        for member in tar.getmembers():\n            member_path = os.path.join(dest_directory, member.name)\n            \n            # Security check: prevent path traversal attacks\n            if not is_safe_path(dest_directory, os.path.abspath(member_path)):\n                raise Exception(f"Attempted path traversal: {member.name}")\n            \n            tar.extract(member, dest_directory)\n\ndef read_file(file_path):\n    """\n    Opens and reads a file, returning its contents.\n    """\n    with open(file_path, 'r') as file_handle:\n        return file_handle.read()\n\ndef write_file(file_path, content):\n    """\n    Opens and writes content to a file.\n    """\n    with open(file_path, 'w') as file_handle:
        file_handle.write(content)

if __name__ == "__main__":
    # Test cases
    test_files = [
        "test1.tar",
        "test2.tar.gz",
        "test3.tar.bz2",
        "test4.tgz",
        "test5.tar"
    ]
    
    for test_file in test_files:
        try:
            extract_tar_archive(test_file, "/tmp/unpack")
            print(f"Successfully extracted: {test_file}")
        except Exception as e:
            print(f"Failed to extract {test_file}: {str(e)}")
