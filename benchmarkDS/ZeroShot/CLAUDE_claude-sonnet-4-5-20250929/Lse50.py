
import zipfile
import os
import tarfile

def extract_archive(archive_path, dest_directory):
    """\n    Extract archive.zip file to /tmp/unpack directory\n    Handles both zip and tar files securely\n    """
    # Create destination directory if it doesn't exist\n    os.makedirs(dest_directory, exist_ok=True)\n    \n    # Normalize the destination path to prevent path traversal\n    dest_directory = os.path.abspath(dest_directory)\n    \n    try:\n        # Try to open as zip file\n        with zipfile.ZipFile(archive_path, 'r') as zip_ref:\n            # Security check: validate all paths before extraction\n            for member in zip_ref.namelist():\n                member_path = os.path.abspath(os.path.join(dest_directory, member))\n                if not member_path.startswith(dest_directory):\n                    raise Exception(f"Path traversal detected: {member}")\n            \n            # Extract all files\n            zip_ref.extractall(dest_directory)\n            return True\n    except zipfile.BadZipFile:\n        # If not a zip file, try as tar file\n        try:\n            with tarfile.open(archive_path, 'r:*') as tar_ref:\n                # Security check: validate all paths before extraction\n                for member in tar_ref.getmembers():\n                    member_path = os.path.abspath(os.path.join(dest_directory, member.name))\n                    if not member_path.startswith(dest_directory):\n                        raise Exception(f"Path traversal detected: {member.name}")\n                \n                # Extract all files safely\n                tar_ref.extractall(dest_directory, filter='data')
                return True
        except Exception as e:
            print(f"Error extracting tar file: {e}")
            return False
    except Exception as e:
        print(f"Error extracting file: {e}")
        return False

if __name__ == "__main__":
    # Test cases
    test_cases = [
        "archive.zip",
        "test1.zip",
        "test2.tar",
        "test3.tar.gz",
        "test4.zip"
    ]
    
    for test_case in test_cases:
        print(f"Attempting to extract: {test_case}")
        result = extract_archive(test_case, "/tmp/unpack")
        if result:
            print(f"Successfully extracted: {test_case}")
        else:
            print(f"Failed to extract: {test_case}")
        print("-" * 50)
